#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>
#include <assert.h>
#include <cmath>
#include <algorithm>

#include "d3du.h"
#include "util.h"
#include "math.h"

static union {
    ID3D11Buffer* buffers[16];
    ID3D11ShaderResourceView* srvs[16];
    ID3D11RenderTargetView* rtvs[16];
} s_no;

struct CubeConstBuf {
    math::mat44 clip_from_world;
    math::vec3 world_down_vector;
    float time_offs;

    math::vec3 light_color_ambient;
    float pad1;
    math::vec3 light_color_key;
    float pad2;
    math::vec3 light_color_fill;
    float pad3;
    math::vec3 light_color_back;
    float pad4;
    math::vec3 light_dir;
    float pad5;
};

struct UpdateConstBuf {
    math::vec3 field_scale;
    float damping;
    math::vec3 field_offs;
    float accel;
    math::vec3 field_sample_scale;
    float vel_scale;
};

static float srgb2lin(float x)
{
    static const float lin_thresh = 0.04045f;
    if (x < lin_thresh)
        return x * (1.0f / 12.92f);
    else
        return std::pow((x + 0.055f) / 1.055f, 2.4f);
}

static math::vec3 srgb_color(int col)
{
    return math::vec3(
        srgb2lin(((col >> 16) & 0xff) / 255.0f),
        srgb2lin(((col >>  8) & 0xff) / 255.0f),
        srgb2lin(((col >>  0) & 0xff) / 255.0f)
    );
}

static void* map_cbuf_typeless(d3du_context* ctx, ID3D11Buffer* buf)
{
    D3D11_MAPPED_SUBRESOURCE mapped;
    HRESULT hr = ctx->ctx->Map(buf, 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
    if (FAILED(hr))
        panic("D3D buffer map failed!\n");
    return mapped.pData;
}

template<typename T>
static T* map_cbuf(d3du_context* ctx, ID3D11Buffer* buf)
{
    return (T*) map_cbuf_typeless(ctx, buf);
}

static void unmap_cbuf(d3du_context* ctx, ID3D11Buffer* buf)
{
    ctx->ctx->Unmap(buf, 0);
}

static ID3D11Buffer* make_cube_inds(ID3D11Device* dev, int num_cubes)
{
    static const USHORT cube_inds[] = {
        0, 2, 1, 3, 7, 2, 6, 0, 4, 1, 5, 7, 4, 6,
    };

    assert(num_cubes * 8 < 65535); // 65535 = prim restart
    USHORT * ind_data = new USHORT[num_cubes * 15];
    for (int i=0; i < num_cubes; i++)
    {
        USHORT * out_ind = ind_data + i*15;
        for (UINT j=0; j < 14; j++)
            out_ind[j] = cube_inds[j] + i*8;
        out_ind[14] = 0xffff;
    }

    ID3D11Buffer* ind_buf = d3du_make_buffer(dev, num_cubes * 15 * sizeof(USHORT),
        D3D11_USAGE_IMMUTABLE, D3D11_BIND_INDEX_BUFFER, ind_data);
    delete[] ind_data;

    return ind_buf;
}

static bool is_pow2(int x)
{
    return x != 0 && (x & (x - 1)) == 0;
}

static float randf()
{
    return 1.0f * rand() / RAND_MAX;
}

static math::vec3 rand_vec3_unit_sphere(float* len_sq_out = nullptr)
{
    math::vec3 v;
    float l;

    do
    {
        v.x = 2.0f * randf() - 1.0f;
        v.y = 2.0f * randf() - 1.0f;
        v.z = 2.0f * randf() - 1.0f;
        l = math::len_sq(v);
    } while (l > 1.0f);

    if (len_sq_out)
        *len_sq_out = l;
    return v;
}

static math::vec3 rand_unit_vec3()
{
    math::vec3 v;
    float l;

    do v = rand_vec3_unit_sphere(&l); while (l == 0.0f);
    return math::rsqrt(l) * v;
}

static int step_idx(int base, int step, int mask)
{
    return (base & ~mask) | ((base + step) & mask);
}

static d3du_tex* make_force_tex(ID3D11Device* dev, int size, float strength, float post_scale)
{
    using namespace math;
    assert(is_pow2(size));

    int stepx = 1, maskx = size - 1;
    int stepy = size, masky = (size - 1) * size;
    int stepz = size*size, maskz = (size - 1) * size * size;
    int nelem = size * size * size;
    vec4* forces = new vec4[nelem];
    
    // create a random vector field
    for (int zo = 0; zo <= maskz; zo += stepz) {
        for (int yo = 0; yo <= masky; yo += stepy) {
            for (int xo = 0; xo <= maskx; xo += stepx) {
                forces[xo + yo + zo] = math::vec4(strength * rand_unit_vec3(), 0.0f);
            }
        }
    }

    // calc divergences
    float* div = new float[nelem];
    float* high = new float[nelem];

    float div_scale = -0.5f / (float)size;

    for (int zo = 0; zo <= maskz; zo += stepz) {
        for (int yo = 0; yo <= masky; yo += stepy) {
            for (int xo = 0; xo <= maskx; xo += stepx) {
                int o = xo + yo + zo;

                div[o] = div_scale * 
                    (
                        forces[step_idx(o, stepx, maskx)].x - forces[step_idx(o, -stepx, maskx)].x +
                        forces[step_idx(o, stepy, masky)].y - forces[step_idx(o, -stepy, masky)].y +
                        forces[step_idx(o, stepz, maskz)].z - forces[step_idx(o, -stepz, maskz)].z
                    );
                high[o] = 0.0f;
            }
        }
    }

    // gauss-seidel iteration to calc density field
    for (int step = 0; step < 40; step++) {
        for (int zo = 0; zo <= maskz; zo += stepz) {
            for (int yo = 0; yo <= masky; yo += stepy) {
                for (int xo = 0; xo <= maskx; xo += stepx) {
                    int o = xo + yo + zo;
                    high[o] =
                        (
                            high[step_idx(o, -stepx, maskx)] + high[step_idx(o, stepx, maskx)] +
                            high[step_idx(o, -stepy, masky)] + high[step_idx(o, stepy, masky)] +
                            high[step_idx(o, -stepz, maskz)] + high[step_idx(o, stepz, maskz)]
                        ) * (1.0f / 6.0f) - div[o];
                }
            }
        }
    }

    // remove gradients from vector field
    float grad_scale = 0.5f * (float)size;
    for (int zo = 0; zo <= maskz; zo += stepz) {
        for (int yo = 0; yo <= masky; yo += stepy) {
            for (int xo = 0; xo <= maskx; xo += stepx) {
                int o = xo + yo + zo;
                vec4* f = forces + o;
                
                f->x = (f->x - grad_scale * (high[step_idx(o, stepx, maskx)] - high[step_idx(o, -stepx, maskx)])) * post_scale;
                f->y = (f->y - grad_scale * (high[step_idx(o, stepy, masky)] - high[step_idx(o, -stepy, masky)])) * post_scale;
                f->z = (f->z - grad_scale * (high[step_idx(o, stepz, maskz)] - high[step_idx(o, -stepz, maskz)])) * post_scale;
            }
        }
    }
    
    d3du_tex* tex = d3du_tex::make3d(dev, size, size, size, 1, DXGI_FORMAT_R32G32B32A32_FLOAT,
        D3D11_USAGE_IMMUTABLE, D3D11_BIND_SHADER_RESOURCE, forces, stepy * sizeof(*forces), stepz * sizeof(*forces));

    delete[] div;
    delete[] high;
    delete[] forces;
    return tex;
}

int main()
{	
    d3du_context* d3d = d3du_init("Momentous", 1280, 720, D3D_FEATURE_LEVEL_10_0);

    char* shader_source = read_file("assets/shader/shaders.hlsl");

    ID3D11VertexShader *update_vs = d3du_compile_and_create_shader(d3d->dev, shader_source,
        "vs_4_0", "UpdateVertShader").vs;
    ID3D11PixelShader *update_pos_ps = d3du_compile_and_create_shader(d3d->dev, shader_source,
        "ps_4_0", "UpdatePosShader").ps;
    ID3D11PixelShader *update_vel_ps = d3du_compile_and_create_shader(d3d->dev, shader_source,
        "ps_4_0", "UpdateVelShader").ps;

    ID3D11VertexShader *cube_vs = d3du_compile_and_create_shader(d3d->dev, shader_source,
        "vs_4_0", "RenderCubeVertexShader").vs;
	ID3D11PixelShader *cube_ps = d3du_compile_and_create_shader(d3d->dev, shader_source,
		"ps_4_0", "RenderCubePixelShader").ps;

    free(shader_source);

    static const UINT kChunkSize = 1024;
    static const UINT kNumCubes = 48 * 1024;
    static const UINT kTexHeight = (kNumCubes + kChunkSize - 1) / kChunkSize;

    ID3D11Buffer* update_const_buf = d3du_make_buffer(d3d->dev, sizeof(UpdateConstBuf),
        D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, NULL);

    ID3D11Buffer* cube_const_buf = d3du_make_buffer(d3d->dev, sizeof(CubeConstBuf),
        D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, NULL);

    ID3D11Buffer* cube_index_buf = make_cube_inds(d3d->dev, kChunkSize);

    ID3D11RasterizerState* raster_state = d3du_simple_raster(d3d->dev, D3D11_CULL_BACK, true, false);
    ID3D11SamplerState* force_sampler = d3du_simple_sampler(d3d->dev, D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP);

    // triple-buffer for position, plus velocity
    d3du_tex* part_tex[4];
    for (int i=0; i < 4; i++)
        part_tex[i] = d3du_tex::make2d(d3d->dev, kChunkSize, kTexHeight, 1, DXGI_FORMAT_R32G32B32A32_FLOAT,
            D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET, NULL, 0);

    d3du_tex* force_tex = make_force_tex(d3d->dev, 32, 1.0f, 0.001f);

    D3D11_VIEWPORT part_vp = d3du_full_tex2d_viewport(part_tex[0]->tex2d);

    int frame = 0;
    unsigned int cur_part = 0;
    int num_cubes = kNumCubes;
    unsigned int spawn_counter = 0;

    while (d3du_handle_events(d3d)) {
        using namespace math;

        static const float part_size = 0.001f;

        vec3 emit_pos(0.0f);
        emit_pos.x = 0.7f * sin(frame * 0.001f);

        // spawn new particles
        {
            static const int kSpawnCount = 256;
            vec4 pos_old[kSpawnCount];
            vec4 pos_new[kSpawnCount];

            for (int i = 0; i < kSpawnCount; i++) {
                vec3 pos = emit_pos + rand_vec3_unit_sphere() * 0.002f;
                vec3 vel = rand_vec3_unit_sphere() * 0.003f;

                pos_old[i] = vec4(pos - vel, part_size);
                pos_new[i] = vec4(pos, part_size);
            }

            // upload
            D3D11_BOX box = { };
            box.left = spawn_counter % kChunkSize;
            box.right = box.left + kSpawnCount;
            box.top = spawn_counter / kChunkSize;
            box.bottom = box.top + 1;
            box.front = 0;
            box.back = 1;
            d3d->ctx->UpdateSubresource(part_tex[(cur_part + 2) % 3]->tex2d, 0, &box, pos_old, 0, 0);
            d3d->ctx->UpdateSubresource(part_tex[cur_part]->tex2d, 0, &box, pos_new, 0, 0);

            spawn_counter = (spawn_counter + kSpawnCount) % num_cubes;
        }

        // set up update constant buffer
        auto update_consts = map_cbuf<UpdateConstBuf>(d3d, update_const_buf);
        update_consts->field_scale = math::vec3(32.0f);
        update_consts->damping = 0.99f;
        update_consts->field_offs = math::vec3(0.0f);
        update_consts->accel = 0.75f;
        update_consts->field_sample_scale = math::vec3(1.0f / 32.0f);
        update_consts->vel_scale = part_size * 6.0f;
        unmap_cbuf(d3d, update_const_buf);

        // update position (potentially several time steps)
        d3d->ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);

        d3d->ctx->VSSetShader(update_vs, NULL, 0);
        d3d->ctx->RSSetViewports(1, &part_vp);

        d3d->ctx->PSSetShader(update_pos_ps, NULL, 0);
        d3d->ctx->PSSetSamplers(0, 1, &force_sampler);
        d3d->ctx->PSSetConstantBuffers(1, 1, &update_const_buf);
        d3d->ctx->PSSetShaderResources(2, 1, &force_tex->srv);
        for (int step=0; step < 1; step++) {
            cur_part = (cur_part + 1) % 3;

            ID3D11ShaderResourceView* srvs[2];
            for (int i=0; i < 2; i++)
                srvs[i] = part_tex[(cur_part + 1 + i) % 3]->srv;

            d3d->ctx->PSSetShaderResources(0, 2, srvs);
            d3d->ctx->OMSetRenderTargets(1, &part_tex[cur_part]->rtv, NULL);
            d3d->ctx->Draw(3, 0);
            d3d->ctx->PSSetShaderResources(0, 2, s_no.srvs);
            d3d->ctx->OMSetRenderTargets(1, s_no.rtvs, NULL);
        }

        // update velocities
        {
            ID3D11ShaderResourceView* srvs[2];
            for (int i=0; i < 2; i++)
                srvs[i] = part_tex[(cur_part + 2 + i) % 3]->srv;

            d3d->ctx->PSSetShader(update_vel_ps, NULL, 0);
            d3d->ctx->PSSetShaderResources(0, 2, srvs);
            d3d->ctx->OMSetRenderTargets(1, &part_tex[3]->rtv, NULL);
            d3d->ctx->Draw(3, 0);
            d3d->ctx->PSSetShaderResources(0, 2, s_no.srvs);
            d3d->ctx->OMSetRenderTargets(1, s_no.rtvs, NULL);
        }

        static const float clear_color[4] = { 0.2f, 0.4f, 0.6f, 1.0f };
        d3d->ctx->ClearDepthStencilView(d3d->depthbuf_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        d3d->ctx->ClearRenderTargetView(d3d->backbuf_rtv, clear_color);

        // back to main render target and viewport
        d3d->ctx->OMSetRenderTargets(1, &d3d->backbuf_rtv, d3d->depthbuf_dsv);
        d3d->ctx->RSSetViewports(1, &d3d->default_vp);

        // set up camera
        vec3 world_cam_pos(0.0f, 0.0f, -0.9f);
        vec3 world_cam_target = emit_pos;
        mat44 view_from_world = mat44::look_at(world_cam_pos, world_cam_target, vec3(0,1,0));

        // projection
        mat44 clip_from_view = mat44::perspectiveD3D(1280.0f / 720.0f, 1.0f, 0.01f, 50.0f);
        mat44 clip_from_world = clip_from_view * view_from_world;

        auto cube_consts = map_cbuf<CubeConstBuf>(d3d, cube_const_buf);
        cube_consts->clip_from_world = clip_from_world;
        cube_consts->world_down_vector = math::vec3(0.0f, 1.0f, 0.0f);
        cube_consts->time_offs = frame * 0.0001f;
        cube_consts->light_color_ambient = srgb_color(0x202020);
        cube_consts->light_color_key = srgb_color(0xc0c0c0);
        cube_consts->light_color_back = srgb_color(0x101040);
        cube_consts->light_color_fill = srgb_color(0x602020);
        cube_consts->light_dir = normalize(vec3(0.0f, -0.7f, -0.3f));
        unmap_cbuf(d3d, cube_const_buf);

        // render cubes
        ID3D11ShaderResourceView* part_pos_srvs[2];
        part_pos_srvs[0] = part_tex[cur_part]->srv;
        part_pos_srvs[1] = part_tex[3]->srv;

        d3d->ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLESTRIP);
        d3d->ctx->IASetIndexBuffer(cube_index_buf, DXGI_FORMAT_R16_UINT, 0);

        d3d->ctx->VSSetShader(cube_vs, NULL, 0);
        d3d->ctx->VSSetShaderResources(0, 2, part_pos_srvs);
        d3d->ctx->VSSetConstantBuffers(0, 1, &cube_const_buf);

        d3d->ctx->RSSetState(raster_state);

        d3d->ctx->PSSetShader(cube_ps, NULL, 0);
        d3d->ctx->PSSetConstantBuffers(0, 1, &cube_const_buf);

        d3d->ctx->DrawIndexedInstanced(kChunkSize * 15, (num_cubes + kChunkSize - 1) / kChunkSize, 0, 0, 0);

        d3d->ctx->VSSetShaderResources(0, 2, s_no.srvs);

        d3du_swap_buffers(d3d, true);
        frame++;
    }

    for (int i=0; i < 4; i++)
        delete part_tex[i];
    delete force_tex;

    update_const_buf->Release();
    cube_const_buf->Release();
    cube_index_buf->Release();
    cube_ps->Release();
    cube_vs->Release();
    update_vs->Release();
    update_pos_ps->Release();
    update_vel_ps->Release();
    raster_state->Release();
    force_sampler->Release();

    d3du_shutdown(d3d);
    return 0;
}