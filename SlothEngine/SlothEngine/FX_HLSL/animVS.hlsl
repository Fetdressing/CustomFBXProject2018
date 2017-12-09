cbuffer boneBuffer  : register (b8)
{
	//Max 96 bones per mesh
	matrix gBoneTransforms[45];
};
//VERTEX SHADER
cbuffer World : register (b0)
{
	matrix View;
	matrix Projection;
	matrix WorldSpace;
	matrix WorldSpaceInv;

	matrix lightView;
	matrix lightProjection;
};

struct SkinnedVertexIn
{
	float3 PosL			 : POSITION;
	float2 Tex			 : TEXCOORD;
	float3 normals		 : NORMAL;
	float3 tangent		 : TANGENT;
	float4 Weights		 : BLENDWEIGHT;
	uint4 BoneIndices	 : BLENDINDICES;
	float pad			 : PADDING;
};

cbuffer CamBuffer : register (b4)
{

	float3 viewPoint;
	float pad;
};

struct VertexOut
{
	float4 PosH : SV_POSITION;
	float2 Tex  : TEXCOORD;
	float4 NormalL    : NORMAL;
	float3 viewDir : TEXCOORD3;

};


VertexOut VS_main(SkinnedVertexIn vin) {

	VertexOut vout;

	 //Init array or else we get strange warnings about SV_POSITION.
	float weights[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	weights[0] = vin.Weights.x;
	weights[1] = vin.Weights.y;
	weights[2] = vin.Weights.z;
	weights[3] = vin.Weights.w;

	float3 posL = float3(0.0f, 0.0f, 0.0f);
	for (int i = 0; i < 4; ++i)
	{
		//Animate vertex positions
		posL += weights[i] * mul(float4(vin.PosL, 1.0f), gBoneTransforms[vin.BoneIndices[i]]); //gBoneTransforms[vin.BoneIndices[i]]).xyz;
	}

	// Transform to homogeneous clip space.
		//vout.PosH = mul(float4(vin.PosL, 1.0f), WorldSpace);
		vout.PosH = float4(posL, 1.0f);
		vout.PosH = mul(vout.PosH, View);
		vout.PosH = mul(vout.PosH, Projection);

	vout.Tex = vin.Tex;

	vout.NormalL = float4(posL.x,posL.y, posL.z,1.0f);
	// Determine the viewing direction based on the position of the camera and the position of the vertex in the world.
	vout.viewDir = viewPoint.xyz;// -worldPosition.xyz;

	return vout;

}