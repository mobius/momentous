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



int main()
{
	d3du_context* d3d = d3du_init("Momentous", 1280, 720, D3D_FEATURE_LEVEL_11_0);
	
	char* shader_source = read_file("assets/shader/simple.hlsl");
	ID3D11VertexShader* simple_vs = d3du_compile_and_create_shader(d3d->dev, shader_source, 
		"vs_5_0", "SimpleVSMain").vs;
	ID3D11PixelShader* simple_ps = d3du_compile_and_create_shader(d3d->dev, shader_source,
		"ps_5_0", "SimplePSMain").ps;
	ID3D11GeometryShader* simple_gs = d3du_compile_and_create_shader(d3d->dev, shader_source,
		"gs_5_0", "SimpleGSMain").gs;

	free(shader_source);

	// aiScene* scene = aiImportFile("assets/mesh/box.obj", 0);
	// aiReleaseImport(scene);



	return 0;
}