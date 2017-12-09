#include "LightHelper.fx"

Texture2D txDiffuse : register(t0);
SamplerState sampWrap: register(s0);



cbuffer LightStruct : register(b0)
{
	DirLight	lDir;
	PointLight pLights[10];
	SpotLight spot;
};

cbuffer MatView : register(b1)
{
	Material gMaterial;
	float3	gEyePos;
	int	hasTexture;
	//int		pad;
};


struct GeoOutPut
{
	
	float4 Pos : SV_POSITION;
	float4 tunormal : NORMAL;
	float2 Tex : TEXCOORD;
	float4 wPos		: POSITION;
	float3 viewDir : TEXCOORD1;

};

float4 PS_main(GeoOutPut input) : SV_Target
{	
		float4 spotColor;
		float4 	textureColor = txDiffuse.Sample(sampWrap, input.Tex);
		float3 Viewpoint = float3(0, 0, 0);
		// Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
		Viewpoint = input.viewDir - input.wPos.xyz;
		int Tex = hasTexture;
		// Normalize the viewing direction vector.
		float dTeye = length(Viewpoint);
		Viewpoint /= dTeye;

		float attenuation = 0;
		float3 lightvec = float3(0, 0, 0);

		//set deafult values
		float4 ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
		float4 spec = float4(0.0f, 0.0f, 0.0f, 0.0f);

		float4 A, D, S;

		for (int i = 0; i < 1; i++) 
		{
			ComputePointLight(gMaterial, pLights[i], input.wPos.xyz, input.tunormal , Viewpoint, A, D, S);

			lightvec = float3(pLights[i].Pos) - float3(input.wPos.xyz);
			attenuation = saturate(1.0f - length(lightvec) / 65);
			ambient +=	A*attenuation;
			diffuse += D *attenuation;
			spec += S *attenuation;
			ambient *= float4(0.6f, 0.6f, 0.6f, 0.6f);
		}


	/*	ComputeDirrLight(lDir, input.lightViewPos, input.wPos.xyz, input.tunormal, Viewpoint, sampClamp, depthMapTexture, color, option7);
		textureColor = txDiffuse.Sample(sampWrap, input.tex);
		color = color*textureColor;
		color = saturate(color);*/
	/*	ComputeSpotLight(gMaterial, spot, input.Pos.xyz, input.tunormal, Viewpoint, spotColor);
		spotColor = spotColor*textureColor;
		spotColor = saturate(spotColor);
*/
		float4 litColor = ambient + diffuse;// +spec;
		if (Tex == 1)
			return float4(saturate(litColor))*textureColor;
		else if (Tex == 0)
			return float4(saturate(litColor));
		else
			return float4(0.0f, 1.0f, 0.0f, 0.0f);





		//// SPECUALAR IS BUGGED ATM
};