//-------------------------------------------------------------------------
// Structs
struct VSIn
{
	float3 pos : POSITION;
	float2 uv : TEXCOORD0; 
};

struct PSIn
{
	float4 pos : SV_Position;
 	float2 uv : TEXCOORD0;
};

//--------------------------------------------------------------------------
// Samplers
SamplerState lin : register(s0);


//--------------------------------------------------------------------------
// Textures & Buffers
Texture2D diffuse : register(t0);

//--------------------------------------------------------------------------
// Const buffers
cbuffer cbChangeEveryFrame : register(b2)
{
	matrix world;
	float4 color;
};


//--------------------------------------------------------------------------
// Vertex Shaders
PSIn SimpleVSMain( VSIn input )
{
 	PSIn output = (PSIn)0;
 	output.pos = mul(world, float4(input.pos, 1.0f));
 	output.uv = input.uv;
 	return output;
}


//--------------------------------------------------------------------------
// Pixel Shaders
float4 SimplePSMain( PSIn input ) : SV_Target
{
	return diffuse.Sample(lin, input.uv) * color;
}