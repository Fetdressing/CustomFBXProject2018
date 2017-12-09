//Tobias Johansson (tojb13), BTH, 2017.
//OH
//I think this should be one of the more simplistic ways to implement a circular buffer.

#include <Windows.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "..\Headers\Reader.h"
#include <assert.h>
#include "../DirectXTK/Inc/SimpleMath.h"
HANDLE hFileMap, controlMap;
void* mData;
void* mControl;
bool exists = false;
size_t numConsumers = 1;

DWORD dwLength;

//FOR MUTEX CREATION
DWORD ms = INFINITE;

Control control;
Header msgHeader;


void Reader::Initizalize(size_t memSizeMegaBytes, size_t delay, size_t msgSize)
{
	//Memsize in actual MiB   HAVE TO BE SAME AS IN MAYA (THE PRODUCER)
	kMemSizeMegaBytes = memSizeMegaBytes;
	//Consumer delay
	kDelay = delay;
	//Message size
	kMsgSize = msgSize;

	tempID = 0;
}

void Reader::createFileMap(size_t memorySize)
{


	hFileMap = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		(DWORD)0,
		DWORD(memorySize),
		(LPCWSTR)"mayaViewerFileMap");


	controlMap = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		(DWORD)0,
		(1024 * 1024),
		(LPCWSTR) "mayaViewerControlMap");

	if (hFileMap == NULL)
	{
		//printf("\nFATAL ERROR");
	}

	if (GetLastError() == ERROR_ALREADY_EXISTS)
	{
		//printf("\nFile Map Already Existed!");
		exists = true;
		//printf("\n\nAutomagically assigned: CONSUMER\n\n");
	}
	else
	{
		//printf("\nI was the first one!\n\nAutomagically assigned: PRODUCER\n");
	}
	//    This means that the file map already existed!, but we still get a handle to it, we share it!
	//    THIS COULD BE USEFUL INFORMATION FOR OUR PROTOCOL.

	//	Create one VIEW of the map with a void pointer
	//    This API Call will create a view to ALL the memory of the File map.

	mData = MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	mControl = MapViewOfFile(controlMap, FILE_MAP_ALL_ACCESS, 0, 0, 0);

	control.head = (size_t*)mControl;
	control.tail = control.head + 1;
	//control.meshesInScene = control.tail + 1;
	//control.lightsInScene = control.meshesInScene + 1;
	//control.camerasInScene = control.lightsInScene + 1;

	if (exists == false)
	{
		*control.head = 0;
		*control.tail = 0;
	}
}

void Reader::releaseStuff()
{
	UnmapViewOfFile((LPCVOID)mData);
	UnmapViewOfFile((LPCVOID)mControl);
	CloseHandle(hFileMap);
	CloseHandle(controlMap);
}

int Reader::Consumer(ID3D11Device* gDevice)
{
	//PRINT RELEVANT INFORMATION about the filemap and the control map
	//printf("fileMap Loc: %d\n\n", &mData);
	//printf("controlMap Loc: %d\n\n", &mControl);


	if (*control.tail != *control.head)
	{
		if ((kMemSizeMegaBytes - *control.tail) > sizeof(msgHeader))	//If there is room for at least the Message Header, Read it!
		{
			//memcpy(Destination pointer to copy to, Source pointer to copy From, Bytes to copy(size of Message))
			memcpy(&msgHeader, (char*)mData + *control.tail, sizeof(msgHeader));	//Read the shared memory Header data, to use in the next lines
		}
		else   //If there is NOT space for the header, its at the end of the buffer. Just reset the tails and read the next header.
		{
			if (*control.head == 0)	//If the head is still at 0, just break out of the func and try again next frame. 
				return 0;

			*control.tail = 0;	//If not, move the tail forth and continue. 
			memcpy(&msgHeader, (char*)mData + *control.tail, sizeof(msgHeader));	//Read the next header and the real message instead
		}

		if (msgHeader.terminateBool)
			PostQuitMessage(0);

		if (msgHeader.dummy == true)	//The message about to be produced didn't fit at the end of buffer. Move the tail to 0
		{
			*control.tail = 0;
			memcpy(&msgHeader, (char*)mData + *control.tail, sizeof(msgHeader));	//Read the next header and the real message instead
		}

		//Switch for different messages. In short a way of knowing wich message to read. Ex Mesh/Transform/Material
		switch (msgHeader.msgType)
		{
		case 1: //Read Mesh message
			readMeshMessage(gDevice);
			break;
		case 2:	//Read Light message
			readLightMessage(gDevice);
			break;
		case 3:	//Read Material message
			readMaterialMessage(gDevice);
			break;
		case 4:	//Read Transfrom message
			readTransformMessage();
			break;
		case 5:	//Read Camera message
			readCameraMessage(gDevice);
			break;
		case 6:
			readTextureMessage(gDevice);
			break;
		default:
			break;
		}

		*control.tail += sizeof(msgHeader);		//If it IS the last one, move the Control tail Forwards
		*control.tail += msgHeader.msgSize;
		*control.tail += msgHeader.padding;

		//printf("Loop Count: %d\n", i);
		//assert((i + 1) == msgHeader.msgID);		//Break the code if the msg.ID is NOT equal to the loop count. //Debugging purposes

		//printf("Message ID: %d\n\n", msgHeader.msgID);
		//assert(tempID == (msgHeader.msgID - 1));		//Break the code if the msg.ID is NOT equal to the loop count. //Debugging purposes

		tempID = msgHeader.msgID;

		testValue = float(msgHeader.faceCount);
		meshStruct.faceCount = int(msgHeader.faceCount);
	}

	return 0;
}

int Reader::main()
{

	//	Consumer(delay, memSizeMegaBytes, nrOfMessages, msgSize);

	//getchar();
	return 0;
}

void Reader::readMeshMessage(ID3D11Device * gDevice) //1
{

	//Read the header for the meshMessage
	//May include: Number of verts , Id as a strin g 
	meshStruct.vertexData.resize(msgHeader.vertCount);
	memcpy(&meshStruct.meshName.lenghtOfString, (char*)mData + *control.tail + sizeof(msgHeader), sizeof(int));
	//Allocate memory for the name string
	meshStruct.meshName.nameString = new char[meshStruct.meshName.lenghtOfString] + 1;
	string test1;
	memcpy(meshStruct.meshName.nameString, (char*)mData + *control.tail + sizeof(msgHeader) + sizeof(int), meshStruct.meshName.lenghtOfString);	//Read the message and store in buff
	memcpy(meshStruct.vertexData.data(), (char*)mData + *control.tail + sizeof(msgHeader) + sizeof(int) + meshStruct.meshName.lenghtOfString, msgHeader.msgSize - sizeof(int) - meshStruct.meshName.lenghtOfString);	//Read the message and store in buff

																																																						//Do the memcopy magic
	meshID = string(meshStruct.meshName.nameString, meshStruct.meshName.lenghtOfString);

	MeshObject tempObj;
	exist = false;
	int test;

	for (int i = 0; i < meshIDs.size(); i++)
	{

		if (meshID == meshIDs[i])
			test = 0;
		else
			test = 1;

		if (test == 0)
		{

			exist = true;
			existingMeshNumber = i;
			break;
		}
		if (test == 1)
		{
			exist = false;
		}
	}

	if (meshIDs.size() == 0)
	{
		exist = false;

	}

	if (exist == false)
	{
		meshIDs.push_back(meshID);
		//Fill the tempObj with mesh data
		tempObj.faceCount = int(msgHeader.faceCount);
		//Fill the vertex buffer
		D3D11_BUFFER_DESC bDesc;
		ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
		bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bDesc.Usage = D3D11_USAGE_DEFAULT;
		bDesc.ByteWidth = UINT(sizeof(Vertex)*(meshStruct.vertexData.size()));
		bDesc.CPUAccessFlags = 0;
		bDesc.MiscFlags = 0;
		bDesc.StructureByteStride = 0;
		//Make the vertex buffer
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = meshStruct.vertexData.data();//<--------
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;
		HRESULT VertexBufferChecker = gDevice->CreateBuffer(&bDesc, &data, &meshVertexBuffer);

		tempObj.vertexBuffer = meshVertexBuffer;
		tempObj.nameID = meshID;

		meshes.push_back(tempObj);

	}
	else if (exist == true)
	{


		//Fill the vertex buffer
		D3D11_BUFFER_DESC bDesc;
		ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
		bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bDesc.Usage = D3D11_USAGE_DEFAULT;
		bDesc.ByteWidth = UINT(sizeof(Vertex)*(meshStruct.vertexData.size()));
		bDesc.CPUAccessFlags = 0;
		bDesc.MiscFlags = 0;
		bDesc.StructureByteStride = 0;
		//Make the vertex buffer
		D3D11_SUBRESOURCE_DATA data;
		data.pSysMem = meshStruct.vertexData.data();//<--------
		data.SysMemPitch = 0;
		data.SysMemSlicePitch = 0;
		HRESULT VertexBufferChecker = gDevice->CreateBuffer(&bDesc, &data, &meshVertexBuffer);

		//Fill the tempObj with mesh data
		tempObj.faceCount = int(msgHeader.faceCount);
		tempObj.nameID = meshID;

		meshes[existingMeshNumber].nameID = tempObj.nameID;
		meshes[existingMeshNumber].vertexBuffer = meshVertexBuffer;
		meshes[existingMeshNumber].faceCount = tempObj.faceCount;

	}


}

void Reader::readLightMessage(ID3D11Device * gDevice) //2
{
}


void Reader::readMaterialMessage(ID3D11Device * gDevice) //3
{
	memcpy(&materialStruct.meshNameLength, (char*)mData + *control.tail + sizeof(msgHeader), sizeof(int));
	materialStruct.meshName = new char[materialStruct.meshNameLength] + 1;
	memcpy(materialStruct.meshName, (char*)mData + *control.tail + sizeof(msgHeader) + sizeof(int), materialStruct.meshNameLength);
	materialStruct.meshNameString = string(materialStruct.meshName, materialStruct.meshNameLength);

	
	memcpy(&materialStruct.materialNameLength, (char*)mData + *control.tail + sizeof(msgHeader) + sizeof(int) + materialStruct.meshNameLength, sizeof(int));
	materialStruct.materialName = new char[materialStruct.materialNameLength] + 1;
	memcpy(materialStruct.materialName, (char*)mData + *control.tail + sizeof(msgHeader) + sizeof(int) + materialStruct.meshNameLength + sizeof(int), materialStruct.materialNameLength);
	materialStruct.materialNameString = string(materialStruct.materialName, materialStruct.materialNameLength);

	memcpy(&materialStruct.matInfoSize, (char*)mData + *control.tail + sizeof(msgHeader) + sizeof(int) + materialStruct.meshNameLength + sizeof(int)+ materialStruct.materialNameLength, sizeof(int));
	memcpy(&materialStruct.materialInfo, (char*)mData + *control.tail + sizeof(msgHeader) + sizeof(int) + materialStruct.meshNameLength + sizeof(int) + materialStruct.materialNameLength + sizeof(int), materialStruct.matInfoSize);


	for (int i = 0; i < meshIDs.size(); i++)
	{
		if (meshes[i].nameID == materialStruct.meshNameString)
		{
			meshes[i].material.Ambient = XMFLOAT4(materialStruct.materialInfo.ka[0], materialStruct.materialInfo.ka[1], materialStruct.materialInfo.ka[2], 1.0f);
			meshes[i].material.Diffuse = XMFLOAT4(materialStruct.materialInfo.kd[0], materialStruct.materialInfo.kd[1], materialStruct.materialInfo.kd[2], 1.0f);
			meshes[i].material.Specular = XMFLOAT4(materialStruct.materialInfo.ks[0], materialStruct.materialInfo.ks[1], materialStruct.materialInfo.ks[2], 1.0f);
			meshes[i].material.SpecPow = materialStruct.materialInfo.specPower;
			meshes[i].materialName = materialStruct.materialNameString;
			meshes[i].hasMaterial = TRUE;
		}
		if (meshes[i].materialName == materialStruct.materialNameString)
		{
			meshes[i].material.Ambient = XMFLOAT4(materialStruct.materialInfo.ka[0], materialStruct.materialInfo.ka[1], materialStruct.materialInfo.ka[2], 1.0f);
			meshes[i].material.Diffuse = XMFLOAT4(materialStruct.materialInfo.kd[0], materialStruct.materialInfo.kd[1], materialStruct.materialInfo.kd[2], 1.0f);
			meshes[i].material.Specular = XMFLOAT4(materialStruct.materialInfo.ks[0], materialStruct.materialInfo.ks[1], materialStruct.materialInfo.ks[2], 1.0f);
			meshes[i].material.SpecPow = materialStruct.materialInfo.specPower;
		}
	}
	int check = 2;
}

void Reader::readTransformMessage() //4
{


	memcpy(&dynamicName.lenghtOfString, (char*)mData + *control.tail + sizeof(msgHeader), sizeof(int));
	//Allocate memory for the name string
	dynamicName.nameString = new char[dynamicName.lenghtOfString] + 1;

	memcpy(dynamicName.nameString, (char*)mData + *control.tail + sizeof(msgHeader) + sizeof(int), dynamicName.lenghtOfString);	//Read the message and store in buff
	memcpy(&transformStruct, (char*)mData + *control.tail + sizeof(msgHeader) + sizeof(int) + dynamicName.lenghtOfString, msgHeader.msgSize - sizeof(int) - dynamicName.lenghtOfString);	//Read the message and store in buff


																																															//Do the memcopy magic
	transformID = string(dynamicName.nameString, dynamicName.lenghtOfString);


	int localExistingMeshNumber;
	bool local = false;
	int test;
	//Check for transform for the meshes
	for (int i = 0; i < meshIDs.size(); i++)
	{

		if (transformID == meshIDs[i])
			test = 0;
		else
			test = 1;

		if (test == 0)
		{

			local = true;
			localExistingMeshNumber = i;
			break;


		}
		if (test == 1)
		{
			local = false;
		}
	}
	if (local == true)
	{
		meshes[localExistingMeshNumber].mayaPos.x = float(transformStruct.pos[0]);
		meshes[localExistingMeshNumber].mayaPos.y = float(transformStruct.pos[1]);
		meshes[localExistingMeshNumber].mayaPos.z = -float(transformStruct.pos[2]);

		meshes[localExistingMeshNumber].mayaScale.x = float(transformStruct.scale[0]);
		meshes[localExistingMeshNumber].mayaScale.y = float(transformStruct.scale[1]);
		meshes[localExistingMeshNumber].mayaScale.z = float(transformStruct.scale[2]);

		meshes[localExistingMeshNumber].mayaRot.m128_f32[0] = -float(transformStruct.rot[0]);
		meshes[localExistingMeshNumber].mayaRot.m128_f32[1] = -float(transformStruct.rot[1]);
		meshes[localExistingMeshNumber].mayaRot.m128_f32[2] = float(transformStruct.rot[2]);
		meshes[localExistingMeshNumber].mayaRot.m128_f32[3] = float(transformStruct.rot[3]);

	}


}

void Reader::readCameraMessage(ID3D11Device * gDevice) //5
{
	memcpy(&cameraStruct, (char*)mData + *control.tail + sizeof(msgHeader), msgHeader.msgSize);	//Read the message and store in buff
	
	tempCam.SetPosition(cameraStruct.pos[0], cameraStruct.pos[1], cameraStruct.pos[2]);

	tempCam.mLook.x = cameraStruct.viewDirection[0];
	tempCam.mLook.y = cameraStruct.viewDirection[1];
	tempCam.mLook.z = cameraStruct.viewDirection[2];

	tempCam.mUp.x = cameraStruct.upVector[0];
	tempCam.mUp.y = cameraStruct.upVector[1];
	tempCam.mUp.z = cameraStruct.upVector[2];

	tempCam.mRight.x = cameraStruct.right[0];
	tempCam.mRight.y = cameraStruct.right[1];
	tempCam.mRight.z = cameraStruct.right[2];

	tempCam.mIsOrtho = cameraStruct.isOrtho;
	tempCam.mOrthoWidthHeight = cameraStruct.orthoWidthHeight;

	//This is the one I used latest
	////Converted
	tempCam.mProj._11 = cameraStruct.projMat[0][0];
	tempCam.mProj._12 = cameraStruct.projMat[0][1];
	tempCam.mProj._13 = cameraStruct.projMat[0][2];
	tempCam.mProj._14 = cameraStruct.projMat[0][3];

	tempCam.mProj._21 = cameraStruct.projMat[1][0];
	tempCam.mProj._22 = cameraStruct.projMat[1][1];
	tempCam.mProj._23 = cameraStruct.projMat[1][2];
	tempCam.mProj._24 = cameraStruct.projMat[1][3];

	tempCam.mProj._31 = -cameraStruct.projMat[2][0];
	tempCam.mProj._32 = -cameraStruct.projMat[2][1];
	tempCam.mProj._33 = -cameraStruct.projMat[2][2];
	tempCam.mProj._34 = -cameraStruct.projMat[2][3];

	tempCam.mProj._41 = cameraStruct.projMat[3][0];
	tempCam.mProj._42 = cameraStruct.projMat[3][1];
	tempCam.mProj._43 = cameraStruct.projMat[3][2];
	tempCam.mProj._44 = cameraStruct.projMat[3][3];
}

void Reader::readTextureMessage(ID3D11Device * gDevice) //6
{

	//memcpy(&msgHeader, (char*)mData + localTail, sizeof(msgHeader));
	memcpy(&textureStruct.filePathLEnght, (char*)mData + *control.tail + sizeof(msgHeader), sizeof(int));
	textureStruct.filePath = new char[textureStruct.filePathLEnght] + 1;
	memcpy(textureStruct.filePath, (char*)mData + *control.tail + sizeof(msgHeader) + sizeof(int), textureStruct.filePathLEnght);
	textureStruct.filePathString = string(textureStruct.filePath, textureStruct.filePathLEnght);


	memcpy(&textureStruct.meshNameLength, (char*)mData + *control.tail + sizeof(msgHeader) + sizeof(int) + textureStruct.filePathLEnght, sizeof(int));
	textureStruct.meshName = new char[textureStruct.meshNameLength] + 1;
	memcpy(textureStruct.meshName, (char*)mData + *control.tail + sizeof(msgHeader) + sizeof(int) + textureStruct.filePathLEnght + sizeof(int), textureStruct.meshNameLength);
	textureStruct.meshNameString = string(textureStruct.meshName, textureStruct.meshNameLength);
	HRESULT texCheck;
	std::wstringstream cls;
	cls << textureStruct.filePathString.c_str();
	std::wstring path = cls.str();
	const wchar_t* finalPath;
	finalPath = path.c_str();
	texCheck = CreateDDSTextureFromFile(gDevice, finalPath, nullptr, &shaderResouce);
	if (texCheck == E_FAIL)
	{
		texCheck = CreateWICTextureFromFile(gDevice, finalPath, nullptr, &shaderResouce);
		if (texCheck == E_FAIL)
			shaderResouce = nullptr;
	}
	/*std::wstringstream cls2;
	cls2 << textureStruct.meshNameString.c_str();
	std::wstring meshNamePath = cls2.str();*/
	///Texture appliance to mesh
	//Loop through all meshes in scence
	//try to find a match via meshname
	string MatwithText;
	for (int i = 0; i < meshIDs.size(); i++)
	{
	

		if (meshes[i].nameID == textureStruct.meshNameString)
		{
			meshes[i].shaderResource1 = shaderResouce;
			materialsWithTextureSR.push_back(shaderResouce);
			meshes[i].hasTexture = TRUE;
			MatwithText = meshes[i].materialName;
			materialsWithTextures.push_back(MatwithText);
		}

		}
	
}