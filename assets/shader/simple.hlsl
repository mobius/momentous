struct VSIn
{
	float4 pos : POSITION;
	float4 color : COLOR;
};

struct VSOut
{
	float4 pos	: SV_Position;
	float4 color : COLOR0;
};

cbuffer SimpleConst : register(b0)
{
	float4x4 mvp;
};

VSOut SimpleVSMain( VSIn in )
{
	VSOut out;
	out.color = in.color;
	out.pos = mul(in.pos, mvp);
}

