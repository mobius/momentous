#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <d3d11.h>
#include <assert.h>
#include <cmath>
#include <algorithm>
#include <d3dx11.h>
#include "util.h"
#include "math.h"
#include "d3du.h"
#include "assimp/cimport.h"

struct SimpleVertex
{
	math::vec3 pos;
	math::vec2 tex;
};

struct CBChangeEveryFrame
{
	math::mat44 world;
	math::vec4  color;
};

int main()
{
	
	d3du_context* d3d = d3du_init("Momentous", 640, 480, D3D_FEATURE_LEVEL_11_0);
	
	char* shader_source = read_file("assets/shader/simple.hlsl");

	ID3DBlob *vsblob, *psblob;
	ID3D11VertexShader* simple_vs = d3du_compile_and_create_shader(d3d->dev, shader_source, 
		"vs_5_0", "SimpleVSMain", vsblob).vs;
	

	ID3D11PixelShader* simple_ps = d3du_compile_and_create_shader(d3d->dev, shader_source,
		"ps_5_0", "SimplePSMain", psblob).ps;

	free(shader_source);

	D3D11_INPUT_ELEMENT_DESC layout[] = 
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	};

	ID3D11InputLayout * inlayout = d3du_make_layout(d3d->dev, layout, 2, vsblob);
	d3d->ctx->IASetInputLayout(inlayout);

	SimpleVertex vertices[] = 
	{
		{ math::vec3( -1.0f, 1.0f, -1.0f ), math::vec2( 0.0f, 0.0f ) },
        { math::vec3( 1.0f, 1.0f, -1.0f ), math::vec2( 1.0f, 0.0f ) },
        { math::vec3( 1.0f, 1.0f, 1.0f ), math::vec2( 1.0f, 1.0f ) },
        { math::vec3( -1.0f, 1.0f, 1.0f ), math::vec2( 0.0f, 1.0f ) },

        { math::vec3( -1.0f, -1.0f, -1.0f ), math::vec2( 0.0f, 0.0f ) },
        { math::vec3( 1.0f, -1.0f, -1.0f ), math::vec2( 1.0f, 0.0f ) },
        { math::vec3( 1.0f, -1.0f, 1.0f ), math::vec2( 1.0f, 1.0f ) },
        { math::vec3( -1.0f, -1.0f, 1.0f ), math::vec2( 0.0f, 1.0f ) },

        { math::vec3( -1.0f, -1.0f, 1.0f ), math::vec2( 0.0f, 0.0f ) },
        { math::vec3( -1.0f, -1.0f, -1.0f ), math::vec2( 1.0f, 0.0f ) },
        { math::vec3( -1.0f, 1.0f, -1.0f ), math::vec2( 1.0f, 1.0f ) },
        { math::vec3( -1.0f, 1.0f, 1.0f ), math::vec2( 0.0f, 1.0f ) },

        { math::vec3( 1.0f, -1.0f, 1.0f ), math::vec2( 0.0f, 0.0f ) },
        { math::vec3( 1.0f, -1.0f, -1.0f ), math::vec2( 1.0f, 0.0f ) },
        { math::vec3( 1.0f, 1.0f, -1.0f ), math::vec2( 1.0f, 1.0f ) },
        { math::vec3( 1.0f, 1.0f, 1.0f ), math::vec2( 0.0f, 1.0f ) },

        { math::vec3( -1.0f, -1.0f, -1.0f ), math::vec2( 0.0f, 0.0f ) },
        { math::vec3( 1.0f, -1.0f, -1.0f ), math::vec2( 1.0f, 0.0f ) },
        { math::vec3( 1.0f, 1.0f, -1.0f ), math::vec2( 1.0f, 1.0f ) },
        { math::vec3( -1.0f, 1.0f, -1.0f ), math::vec2( 0.0f, 1.0f ) },

        { math::vec3( -1.0f, -1.0f, 1.0f ), math::vec2( 0.0f, 0.0f ) },
        { math::vec3( 1.0f, -1.0f, 1.0f ), math::vec2( 1.0f, 0.0f ) },
        { math::vec3( 1.0f, 1.0f, 1.0f ), math::vec2( 1.0f, 1.0f ) },
        { math::vec3( -1.0f, 1.0f, 1.0f ), math::vec2( 0.0f, 1.0f ) },
	};

	ID3D11Buffer* vbuffer = d3du_make_buffer(d3d->dev, sizeof(SimpleVertex)*24, D3D11_USAGE_DEFAULT, D3D11_BIND_VERTEX_BUFFER, vertices);

	UINT stride = sizeof(SimpleVertex);
	UINT offset = 0;
	d3d->ctx->IASetVertexBuffers(0, 1, &vbuffer, &stride, &offset);

	WORD indices[] = 
	{
		3,1,0,
        2,1,3,

        6,4,5,
        7,4,6,

        11,9,8,
        10,9,11,

        14,12,13,
        15,12,14,

        19,17,16,
        18,17,19,

        22,20,21,
        23,20,22
	};

	ID3D11Buffer* ibuffer = d3du_make_buffer(d3d->dev, sizeof(WORD)*36, D3D11_USAGE_DEFAULT, D3D11_BIND_INDEX_BUFFER, indices);

	d3d->ctx->IASetIndexBuffer(ibuffer, DXGI_FORMAT_R16_UINT, 0);
	d3d->ctx->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	ID3D11Buffer* cbChangeEveryFrame = d3du_make_buffer(d3d->dev, sizeof(CBChangeEveryFrame), D3D11_USAGE_DYNAMIC, D3D11_BIND_CONSTANT_BUFFER, NULL);
	ID3D11ShaderResourceView* texSV;
	D3DX11CreateShaderResourceViewFromFile(d3d->dev, "assets/tex/seafloor.dds", NULL, NULL, &texSV, NULL );
	//d3du_tex* tex = d3du_tex::make2d(d3d->dev, 256, 256, 1, DXGI_FORMAT_B8G8R8X8_UNORM, D3D11_USAGE_DEFAULT, D3D11_BIND_SHADER_RESOURCE, NULL, 0);
	ID3D11SamplerState* linear = d3du_simple_sampler(d3d->dev, D3D11_FILTER_MIN_MAG_LINEAR_MIP_POINT, D3D11_TEXTURE_ADDRESS_WRAP);
	ID3D11RasterizerState* raster = d3du_simple_raster(d3d->dev, D3D11_CULL_BACK, true, false);
    d3d->ctx->RSSetViewports(1, &d3d->default_vp);
	
	while(d3du_handle_events(d3d))
	{
		using namespace math;

		static const float clear_color[4] = { 0.2f, 0.4f, 0.6f, 1.0f };
        d3d->ctx->ClearDepthStencilView(d3d->depthbuf_dsv, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
        d3d->ctx->ClearRenderTargetView(d3d->backbuf_rtv, clear_color);

        d3d->ctx->OMSetRenderTargets(1, &d3d->backbuf_rtv, d3d->depthbuf_dsv);

        vec3 world_camera_pos(0.0f, 0.0f, -6.f);
        vec3 world_camera_target(0.0f, 0.0f, 0.0f);
        mat44 view_from_world = mat44::look_at(world_camera_pos, world_camera_target, vec3(0, 1, 0));

        mat44 clip_from_view = mat44::perspectiveD3D(640.f/480.f, 1.0f, 0.01f, 100.0f);
        mat44 clip_from_world = clip_from_view * view_from_world;

        auto constFrame = d3du_map_cbuf<CBChangeEveryFrame>(d3d, cbChangeEveryFrame);
        constFrame->world = clip_from_world;
        constFrame->color = vec4(1.0, 0.0, 0.0, 1.0);
        d3du_unmap_cbuf(d3d, cbChangeEveryFrame);

        d3d->ctx->VSSetShader(simple_vs, NULL, 0);
        d3d->ctx->VSSetConstantBuffers(0, 1, &cbChangeEveryFrame);
        d3d->ctx->RSSetState(raster);
        d3d->ctx->PSSetShader(simple_ps, NULL, 0);
        d3d->ctx->PSSetConstantBuffers(0, 1, &cbChangeEveryFrame);
        d3d->ctx->PSSetShaderResources(0, 1, &texSV);
        d3d->ctx->PSSetSamplers(0, 1, &linear);
        d3d->ctx->DrawIndexed(36, 0, 0);

        d3du_swap_buffers(d3d, true);
		Sleep(30);
	}

	psblob->Release();
	vsblob->Release();
	d3du_shutdown(d3d);

	return 0;
}
