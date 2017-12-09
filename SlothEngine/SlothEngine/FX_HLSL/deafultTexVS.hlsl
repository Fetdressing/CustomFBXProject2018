//VERTEX SHADER
cbuffer World : register (b0)
{
	matrix View;
	matrix Projection;
	matrix WorldSpace;
	matrix WorldSpaceInv;
};
cbuffer CamBuffer : register (b4)
{

	float3 viewPoint;
	float pad;
};

struct VS_IN
{
	float3 Pos : POSITION;
	float3 tunormal : NORMAL;
	float2 Tex : TEXCOORD;
};

struct VS_OUT
{
	//LightViewPos for shadow calc

	float4 Pos : SV_POSITION;
	float4 tunormal : NORMAL;
	float2 Tex : TEXCOORD;
	float4 wPos		: POSITION;
	float3 viewDir : TEXCOORD1;
};

//-----------------------------------------------------------------------------------------
// VertexShader: VSScene
//-----------------------------------------------------------------------------------------
VS_OUT VS_main(VS_IN input)
{
	VS_OUT output = (VS_OUT)0;

	float4 outpos = float4(input.Pos, 1.0f);
		outpos = mul(outpos, WorldSpace);
		outpos = mul(outpos, View);
		outpos = mul(outpos, Projection);

	output.Pos = outpos;
	output.wPos = mul(float4(input.Pos, 1.0f), WorldSpace);
	output.Tex = input.Tex;
	output.tunormal = float4(mul(input.tunormal, (float3x3)WorldSpace),1.0f);
	output.tunormal = normalize(output.tunormal);
	output.viewDir = viewPoint.xyz;
	return output;
}