//-------------------------------------------------------------------------
// Structs
struct VSIn
{
	float4 pos : POSITION;
	float2 uv : TEXCOORD0; 
};

struct PSIn
{
	float4 pos : SV_Position;
 	float2 uv : TEXCOORD0;
};

//--------------------------------------------------------------------------
// Samplers
SamplerState linear : register(s0);


//--------------------------------------------------------------------------
// Textures & Buffers
Texture2D diffuse : register(t0);

//--------------------------------------------------------------------------
// Const buffers
cbuffer cbNeverChange : register(b0)
{
	matrix view;
};

cbuffer cbChangeOnResize : register(b1)
{
	matrix proj;
};

cbuffer cbChangeEveryFrame : register(b2)
{
	matrix world;
	float4 color;
};


//--------------------------------------------------------------------------
// Vertex Shaders
PSIn SimpleVSMain( VSIn in )
{
 	PSIn out = (PSIn)0;
 	out.pos = mul(in.pos, world);
 	out.pos = mul(out.pos, view);
 	out.pos = mul(out.pos, proj);
 	out.uv = in.uv;
}


//--------------------------------------------------------------------------
// Pixel Shaders
float4 SimplePSMain( PSIn in ) : SV_Target
{
	return diffuse.Sample(linear, input.uv) * color;
}