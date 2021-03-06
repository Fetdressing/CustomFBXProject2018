#pragma once
#ifndef READER_H
#define READER_H
#endif
#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <iostream>
#include <fstream>
#include <sstream>

#include <vector>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <DirectXMathVector.inl>
#include <d3d11.h>
#include <d3dcompiler.h>
#include "../DirectXTK/Inc/DDSTextureLoader.h"
#include "../DirectXTK/Inc/WICTextureLoader.h"

#include "..\Headers\MeshObject.h"
#include "..\Headers\Camera.h"

using namespace std;
struct Control	//Shared Control, in the control file map (controlFileMapPointer).
{
public:
	//size_t* numConsumers = 0;
	size_t* head;
	size_t* tail;
	//size_t* meshesInScene;
	//size_t* lightsInScene;
	//size_t* camerasInScene;
};

struct Header
{
public:
	bool dummy = false;	//bool because its cool
	size_t msgID = 0;
	size_t padding = 0;
	size_t msgSize = 0;
	size_t msgType = 0;		//Most important to define how to read message
	size_t vertCount = 0;
	size_t faceCount = 0;
	bool terminateBool = false;
};

struct vec3 {
public:
	float x, y, z;
};

struct vec2 {
public:
	float u, v;

};

struct Vertex
{
public:
	vec3 vPos;
	vec3 vNor;
	vec2 vUV;
};

struct Name
{
	char*  nameString;
	int lenghtOfString;
};

//1
struct Mesh
{
public:
	Name meshName;
	std::vector<Vertex> vertexData;
	int faceCount;
};

//2
struct Light
{
public:
	//Lightdata....
}; 


//3
//struct Materialinfo
//{
//public:
//	float kd[3];
//	float ka[3];
//	float ks[3];
//	float specPower;
//};
struct MayaMaterial
{
public:
	char *meshName;
	char *materialName;
	string meshNameString;
	int meshNameLength;
	string materialNameString;
	int materialNameLength;
	int matInfoSize;
	Materialinfo materialInfo;
}; 

//4
struct Transform
{
public:

	double scale[3]; //x,y,z
	float pos[3]; //x,y,z
	double rot[4]; //x,y,z,w  Rotation as a quaternion
	float matrix[16]; //Tranformation matrix
};


//5
struct MayaCamera
{
public:
	float pos[3]; //x, y, z position of the camera
	float viewDirection[3];	//AKA LookAt
	float upVector[3];
	float right[3];
	bool isOrtho;
	double orthoWidthHeight;
	float projMat[4][4];
	//double rot[4]; //Quaternion rotation of the camera (x, y, z, w)
};


//6
struct Texture
{
public:
	//string filePath;
	char *meshName;
	char *filePath;
	string filePathString;
	string meshNameString;
	int filePathLEnght;
	int meshNameLength;
};



class Reader
{
public:
	size_t localTail = 0;
	//Testing struct with sent transformdata from maya
	Transform transformStruct;
	Texture textureStruct;
	//Testing struct with sent transformdata from maya
	Mesh meshStruct;
	Name dynamicName;
	//Vector that will contain all meshes from maya and a vector with the id�s
	vector<MeshObject> meshes;
	vector<string> meshIDs;
	vector<string> materialsWithTextures;
	vector<ID3D11ShaderResourceView*> materialsWithTextureSR;
	ID3D11ShaderResourceView*   shaderResouce = nullptr;
	bool exist;
	int existingMeshNumber;
	string meshID;
	string transformID;
	ID3D11Buffer* meshVertexBuffer = nullptr;

	MayaMaterial materialStruct;
	Light lightStruct;
	MayaCamera cameraStruct;
	Camera tempCam;
	//Memsize in actual MiB   HAVE TO BE SAME AS IN MAYA (THE PRODUCER)
	size_t kMemSizeMegaBytes;
	//Consumer delay
	size_t kDelay;
	//Message size
	size_t kMsgSize;

	size_t tempID;

	float testValue = 0.0f;
	//Class functions

public:
	void Initizalize(size_t memSizeMegaBytes, size_t delay, size_t msgSize);

	void createFileMap(size_t memorySize);

	void releaseStuff();

	int Consumer(ID3D11Device* gDevice);

	int main();

	//Message readers
	void readMeshMessage(ID3D11Device* gDevice);		//1
	void readLightMessage(ID3D11Device* gDevice);		//2
	void readMaterialMessage(ID3D11Device* gDevice);	//3
	void readTransformMessage();						//4
	void readCameraMessage(ID3D11Device* gDevice);		//5
	void readTextureMessage(ID3D11Device* gDevice);		//6

	
};