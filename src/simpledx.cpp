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
#include "assimp/cimport.h"

struct SimpleVertex
{
	math::vec3 pos;
	math::vec2 tex;
};

struct CBNeverChanges
{
	math::mat44 view;
};

struct CBChangeOnResize
{
	math::mat44 proj;
};

struct CBChangeEveryFrame
{
	math::mat44 world;
	math::vec4  color;
};

int main()
{
	
	d3du_context* d3d = d3du_init("Momentous", 1280, 720, D3D_FEATURE_LEVEL_11_0);
	
	char* shader_source = read_file("assets/shader/simple.hlsl");

	ID3DBlob* vsblob, vsblob;
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

	ID3D11InputLayout * inlayout = d3du_create_layout(d3d->dev, layout, 2, vsblob);
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
	d3d->dev->IASetVertexBuffer(0, 1, &vbuffer, &stride, &offset);

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

	d3d->dev->IASetIndexBuffer(ibuffer, DXGI_FORMAT_R16_UINT, 0);
	d3d->dev->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	
	
	return 0;
}
