#include "LightHelper.fx"

Texture2D txDiffuse : register(t0);
SamplerState sampAni: register(s0);


struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex : TEXCOORD;
	float4 NormalL : NORMAL;
	float3 viewDir : TEXCOORD3;

};

float4 PS_main(VertexOut input) : SV_Target
{

	float4 diffuse = txDiffuse.Sample(sampAni, input.Tex);

	float3 toEye = input.viewDir - input.PosH.xyz;

	// Cache the distance to the eye from this surface point.
	float distToEye = length(toEye);
	// Normalize.
	toEye /= distToEye;

	float fogLerp = saturate((distToEye - 15.0f) / 175.0f);

	// Blend the fog color and the lit color.
	float4 litColor = lerp(diffuse, float4(0.2f,0.0f,0.0f,1.0f), fogLerp);
	litColor.a = diffuse.a;
//	return input.NormalL;
	return diffuse;
	return float4(0.8f,0.0f,0.0f,0.0f);



};