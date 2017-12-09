//#include "BINimporter.h"
//#include "string"
//void BINimporter::ImportBIN(ID3D11Device* gDevice, char* fileName){
//	ifstream slothFile;
//	slothFile.open(fileName, ifstream::binary);
//
//	FileInfo fileInfo;
//
//
//	// IF MESH MESSAGE
//		memcpy((char*)filemap,&meshInfo.id, sizeof(int));
//		memcpy((char*)filemap, &meshInfo.nrPos, sizeof(int));
//		memcpy((char*)filemap, &meshInfo.nrNor, sizeof(int));
//		memcpy((char*)filemap, &meshInfo.nrUV, sizeof(int));
//		memcpy((char*)filemap, &meshInfo.nrFaces, sizeof(int));
//
//
//		int nrOfVertexPositions = meshInfo.nrPos;
//		int nrOfVertexNormals = meshInfo.nrNor;
//		int nrOfVertexUVs = meshInfo.nrUV;
//		int nrOfVertexTangents = meshInfo.nrTangents;
//		int nrOfFaces = meshInfo.nrFaces;
//
//		ID3D11Buffer* meshVertexBuffer;
//
//
//		//Normal vector
//		vector<VertexData> vertecies;
//




//
//		vector<FaceData> faces;
//		vector<XMFLOAT3> vertexPos;
//		vector<XMFLOAT3> vertexNor;
//		vector<XMFLOAT2> vertexUV;
//
//	
//		//ladda in vertexvärden för meshen
//		for (int p = 0; p < nrOfVertexPositions; p++){
//			XMFLOAT3 tempPosStorage;
//
//			memcpy((char*)filemap, &tempPosStorage.x, sizeof(float));
//			memcpy((char*)filemap, &tempPosStorage.y, sizeof(float));
//			memcpy((char*)filemap, &tempPosStorage.z, sizeof(float));
//			tempPosStorage.z = tempPosStorage.z*-1;
//			vertexPos.push_back(tempPosStorage);
//		}
//
//		for (int p = 0; p < nrOfVertexNormals; p++){
//			XMFLOAT3 tempNorStorage;
//
//			memcpy((char*)filemap, &tempNorStorage.x, sizeof(float));
//			memcpy((char*)filemap, &tempNorStorage.y, sizeof(float));
//			memcpy((char*)filemap, &tempNorStorage.z, sizeof(float));
//			tempNorStorage.x = tempNorStorage.x*(1);
//			tempNorStorage.y = tempNorStorage.y*(1);
//			tempNorStorage.z = tempNorStorage.z*(-1);
//			vertexNor.push_back(tempNorStorage);
//		}
//
//		for (int p = 0; p < nrOfVertexUVs; p++){
//			XMFLOAT2 tempUVStorage;
//
//			memcpy((char*)filemap, &tempUVStorage.x, sizeof(float));
//			memcpy((char*)filemap, &tempUVStorage.y, sizeof(float));
//			tempUVStorage.y = tempUVStorage.y*-1;
//			vertexUV.push_back(tempUVStorage);
//		}
//
//
//
//		//ladda in indeciesen och skapa vertiserna
//		Int4 tempVerIndex;
//		VertexData tempVertex;
//		FaceData tempFaceData;
//
//		//BUILD FACE DATA
//		for (int y = 0; y < nrOfFaces; y++){
//
//			//Get indicies for each face //Maybe we can do this in the .mll ?
//			memcpy((char*)filemap, &tempVerIndex.x, sizeof(int));
//			memcpy((char*)filemap, &tempVerIndex.y, sizeof(int));
//			memcpy((char*)filemap, &tempVerIndex.z, sizeof(int));
//			memcpy((char*)filemap, &tempVerIndex.w, sizeof(int));
//
//			tempFaceData.indexPos[0] = tempVerIndex.x;
//			tempFaceData.indexUV[0] = tempVerIndex.y;
//			tempFaceData.indexNor[0] = tempVerIndex.z;
//			tempFaceData.indexTangent[0] = tempVerIndex.w;
//
//			memcpy((char*)filemap, &tempVerIndex.x, sizeof(int));
//			memcpy((char*)filemap, &tempVerIndex.y, sizeof(int));
//			memcpy((char*)filemap, &tempVerIndex.z, sizeof(int));
//			memcpy((char*)filemap, &tempVerIndex.w, sizeof(int));
//
//			tempFaceData.indexPos[1] = tempVerIndex.x;
//			tempFaceData.indexUV[1] = tempVerIndex.y;
//			tempFaceData.indexNor[1] = tempVerIndex.z;
//			tempFaceData.indexTangent[1] = tempVerIndex.w;
//
//			memcpy((char*)filemap, &tempVerIndex.x, sizeof(int));
//			memcpy((char*)filemap, &tempVerIndex.y, sizeof(int));
//			memcpy((char*)filemap, &tempVerIndex.z, sizeof(int));
//			memcpy((char*)filemap, &tempVerIndex.w, sizeof(int));
//
//			tempFaceData.indexPos[2] = tempVerIndex.x;
//			tempFaceData.indexUV[2] = tempVerIndex.y;
//			tempFaceData.indexNor[2] = tempVerIndex.z;
//			tempFaceData.indexTangent[2] = tempVerIndex.w;
//
//			faces.push_back(tempFaceData);
//
//			//Create the face (vertiserna)!
//			tempVertex.vertPos = vertexPos[faces[y].indexPos[2] - 1];
//			tempVertex.vertUV = vertexUV[faces[y].indexUV[2] - 1];
//			tempVertex.vertNor = vertexNor[faces[y].indexNor[2] - 1];
//
//			vertecies.push_back(tempVertex); //vertex 1 i triangeln
//
//			tempVertex.vertPos = vertexPos[faces[y].indexPos[1] - 1];
//			tempVertex.vertUV = vertexUV[faces[y].indexUV[1] - 1];
//			tempVertex.vertNor = vertexNor[faces[y].indexNor[1] - 1];
//
//			vertecies.push_back(tempVertex); //vertex 2 i triangeln
//
//			tempVertex.vertPos = vertexPos[faces[y].indexPos[0] - 1];
//			tempVertex.vertUV = vertexUV[faces[y].indexUV[0] - 1];
//			tempVertex.vertNor = vertexNor[faces[y].indexNor[0] - 1];
//
//			vertecies.push_back(tempVertex); //vertex 3 i triangeln
//		}
//		//Create vertex buffer with standard data
//		D3D11_BUFFER_DESC bDesc;
//		ZeroMemory(&bDesc, sizeof(D3D11_BUFFER_DESC));
//		bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//		bDesc.Usage = D3D11_USAGE_DEFAULT;
//		bDesc.ByteWidth = (UINT)(sizeof(VertexData)*(vertecies.size()));
//		bDesc.CPUAccessFlags = 0;
//		bDesc.MiscFlags = 0;
//		bDesc.StructureByteStride = 0;
//
//		D3D11_SUBRESOURCE_DATA data;
//		data.pSysMem = vertecies.data();//<--------
//		data.SysMemPitch = 0;
//		data.SysMemSlicePitch = 0;
//		HRESULT VertexBufferChecker = gDevice->CreateBuffer(&bDesc, &data, &meshVertexBuffer);
//
//		//Create animation vertex buffer with wheights and influences
//		D3D11_BUFFER_DESC bDesc2;
//		ZeroMemory(&bDesc2, sizeof(D3D11_BUFFER_DESC));
//		bDesc2.BindFlags = D3D11_BIND_VERTEX_BUFFER;
//		bDesc2.Usage = D3D11_USAGE_DEFAULT;
//		bDesc2.ByteWidth = (UINT)(sizeof(VertexData2)*(vertecies2.size()));
//		bDesc2.CPUAccessFlags = 0;
//		bDesc2.MiscFlags = 0;
//		bDesc2.StructureByteStride = 0;
//
//		D3D11_SUBRESOURCE_DATA data2;
//		data2.pSysMem = vertecies2.data();//<--------
//		data2.SysMemPitch = 0;
//		data2.SysMemSlicePitch = 0;
//		VertexBufferChecker = gDevice->CreateBuffer(&bDesc2, &data2, &meshVertexBuffer2);
//
//
//
//		verPos.clear();
//		verNor.clear();
//		verUV.clear();
//		verTangent.clear();
//		faces.clear();
//
//		float centerX, centerY, centerZ, extentX, extentY, extentZ;
//
//		slothFile.read((char*)&centerX, sizeof(float));
//		slothFile.read((char*)&centerY, sizeof(float));
//		slothFile.read((char*)&centerZ, sizeof(float));
//		slothFile.read((char*)&extentX, sizeof(float));
//		slothFile.read((char*)&extentY, sizeof(float));
//		slothFile.read((char*)&extentZ, sizeof(float));
//
//		//MATERIAL!!!!!!
//		XMFLOAT4 KD;
//		XMFLOAT4 KA;
//		XMFLOAT4 KS;
//		float specularPowah, trans;
//
//		slothFile.read((char*)&KD.x, sizeof(float));
//		slothFile.read((char*)&KD.y, sizeof(float));
//		slothFile.read((char*)&KD.z, sizeof(float));
//		KD.w = 1.0f;
//
//		slothFile.read((char*)&KA.x, sizeof(float));
//		slothFile.read((char*)&KA.y, sizeof(float));
//		slothFile.read((char*)&KA.z, sizeof(float));
//		KA.w = 1.0f;
//
//		slothFile.read((char*)&KS.x, sizeof(float));
//		slothFile.read((char*)&KS.y, sizeof(float));
//		slothFile.read((char*)&KS.z, sizeof(float));
//		KS.w = 1.0f;
//
//		slothFile.read((char*)&specularPowah, sizeof(float));
//		slothFile.read((char*)&trans, sizeof(float));
//
//
//		int nrOfKeyFrames;
//		slothFile.read((char*)&nrOfKeyFrames, sizeof(int));
//
//		//Animation
//		int jointnmr;
//		int  parentjointnmr;
//		float boneMatrix[16];
//		XMFLOAT4X4 boneM;
//  		
//		keyframe frame1;
//		vector<int> jointNumbers;
//		vector<int> parentjointNumbers;
//		vector<XMFLOAT4X4> boneMatrixes;
//
//		for (int u = 0; u < meshInfo.nrBones; u++) {
//
//			slothFile.read((char*)&jointnmr, sizeof(int));
//			slothFile.read((char*)&parentjointnmr, sizeof(int));
//			jointNumbers.push_back(jointnmr);
//			parentjointNumbers.push_back(parentjointnmr);
//			parentArray.push_back(parentjointnmr);
//			
//			for (int u2 = 0; u2 < 16; u2++) {
//					slothFile.read((char*)&boneMatrix[u2], sizeof(float));
//					
//			}
//			boneM._11 = boneMatrix[0];
//			boneM._12 = boneMatrix[1];
//			boneM._13 = boneMatrix[2];
//			boneM._14 = boneMatrix[3];
//			boneM._21 = boneMatrix[4];
//			boneM._22 = boneMatrix[5];
//			boneM._23 = boneMatrix[6];
//			boneM._24 = boneMatrix[7];
//			boneM._31 = boneMatrix[8];
//			boneM._32 = boneMatrix[9];
//			boneM._33 = boneMatrix[10];
//			boneM._34 = boneMatrix[11];
//			boneM._41 = boneMatrix[12];
//			boneM._42 = boneMatrix[13];
//			boneM._43 = boneMatrix[14];
//			boneM._44 = boneMatrix[15];
//
//			boneMatrixArray.push_back(boneM);
//			boneMatrixes.push_back(boneM);
//			for (int u3 = 0; u3 < 30; u3++) {
//
//				slothFile.read((char*)&frame1.time, sizeof(float));
//
//				slothFile.read((char*)&frame1.pos.x, sizeof(float));
//				slothFile.read((char*)&frame1.pos.y, sizeof(float));
//				slothFile.read((char*)&frame1.pos.z, sizeof(float));
//	
//				slothFile.read((char*)&frame1.scale.x, sizeof(float));
//				slothFile.read((char*)&frame1.scale.y, sizeof(float));
//				slothFile.read((char*)&frame1.scale.z, sizeof(float));
//
//				slothFile.read((char*)&frame1.quat.x, sizeof(float));
//				slothFile.read((char*)&frame1.quat.y, sizeof(float));
//				slothFile.read((char*)&frame1.quat.z, sizeof(float));
//				slothFile.read((char*)&frame1.quat.w, sizeof(float));
//
//		
//
//				//correct for directx coordinate system
//				frame1.pos.z = -frame1.pos.z;
//
//				//frame1.scale.z = -frame1.scale.z;
//				
//			
//				frame1.quat.x = -frame1.quat.x;
//				frame1.quat.y = -frame1.quat.y;
//				frame1.quat.z = frame1.quat.z;
//				frame1.quat.w = frame1.quat.w;
//			
//
//				keyframeArray.push_back(frame1);
//			}
//	
//		}
//
//		MatInfo material(KA, KD, KS, specularPowah);
//
//
//		//adda dem på listorna
//		if (meshType == 0 || meshType < 0 || meshType == 2){ //static
//			BoundingBox bTemp;
//			bTemp.Center = XMFLOAT3(centerX, centerY, -centerZ);
//			bTemp.Extents = XMFLOAT3(extentX, extentY, extentZ);
//
//			GameObjects* tempP = new GameObjects(meshVertexBuffer, bTemp, false, bTemp.Center, true, false);
//			GameObjects* tempP2 = new GameObjects(meshVertexBuffer2, bTemp, false, bTemp.Center, true, false);
//			//TEST MED FRUSTUM CULLING
//			tempP->SetStatic(true);
//			tempP2->SetStatic(true);
//
//			if (centerX < -1.2f){
//			
//				//tempP->isTransparent = true;
//			}
//			else{
//				tempP->SetStatic(false);
//				tempP->isTransparent = true; //bara för att testa och separera objekt, ta bort detta sen och ersätt med en import där man kan välja vad det ska va i maya
//			}
//			//****************************
//
//			tempP->material.Diffuse = KD;
//			tempP->material.Ambient = KA;
//			tempP->material.Specular = KS;
//			tempP->material.SpecPow = specularPowah;
//			tempP->nrElements = nrOfFaces;
//			tempP->indexT = mesnNumberInt;
//
//
//			tempP2->material.Diffuse = KD;
//			tempP2->material.Ambient = KA;
//			tempP2->material.Specular = KS;
//			tempP2->material.SpecPow = specularPowah;
//			tempP2->nrElements = nrOfFaces;
//			tempP2->indexT = mesnNumberInt;
//
//			mesnNumberInt++;
//			binObj.push_back(tempP);
//			binObjAnim.push_back(tempP2);
//			if (tempP->isTransparent == true)
//				transparentObj.push_back(tempP);
//		}
//
//
//
//	}
//
//	//lights
//
//
//	slothFile.close();
//
//	int cp = 0;
//	for (int f = 0; f < testTexNameArray.size(); f++){
//
//		const char* cmpString3 = testTexNameArray[f].c_str();
//		if (f == 0){
//			std::string finalTexPath = cmpString3;
//
//			texNameArray.push_back(finalTexPath.c_str());
//		}
//		else if (f > 0){
//			std::string finalTexPath1;
//			for (int f2 = 0; f2 < texNameArray.size(); f2++){
//
//				int t2 = 0;
//				if (cmpString3 != texNameArray[f2]){
//
//					t2 = 1;
//					finalTexPath1 = cmpString3;
//					for (int f3 = 0; f3 < texNameArray.size(); f3++){
//						if (cmpString3 == texNameArray[f3])
//							t2 = 0;
//
//
//					}
//					if (f2 == texNameArray.size() - 1 && t2 == 1){
//						finalTexPath1 = cmpString3;
//						texNameArray.push_back(finalTexPath1.c_str());
//						cp++;
//					}
//				}
//			}
//		}
//	}
//	for (int f = 0; f < testTexNameArray.size(); f++){
//		const char* cmpString3 = testTexNameArray[f].c_str();
//		for (int f5 = 0; f5 < texNameArray.size(); f5++){
//			if (cmpString3 == texNameArray[f5]){
//				indexArray.push_back(f5);
//			}
//
//		}
//	}
//
//}
//
//
//void BINimporter::ImportAnimBIN(ID3D11Device* gDevice, char* fileName) {
//
//	ifstream animFile;
//	animFile.open(fileName, ifstream::binary);
//
//	FileInfo fileInfo;
//	MeshInfo meshInfo;
//
//		int nrOfKeyFrames;
//		int nrOfBones;
//		animFile.read((char*)&nrOfBones, sizeof(int));
//		animFile.read((char*)&nrOfKeyFrames, sizeof(int));
//
//		//Animation
//		keyframe frame1;
//
//		for (int u = 0; u < nrOfBones; u++) {
//
//			animFile.read((char*)&frame1.pos.x, sizeof(float));
//			animFile.read((char*)&frame1.pos.y, sizeof(float));
//			animFile.read((char*)&frame1.pos.z, sizeof(float));
//
//
//			for (int i = 0; i < nrOfKeyFrames; i++) {
//
//				animFile.read((char*)&frame1.time, sizeof(float));
//
//
//				if (u == 0) {
//					animFile.read((char*)&frame1.pos.y, sizeof(float));
//				}
//			
//
//				animFile.read((char*)&frame1.quat.x, sizeof(float));
//				animFile.read((char*)&frame1.quat.y, sizeof(float));
//				animFile.read((char*)&frame1.quat.z, sizeof(float));
//				animFile.read((char*)&frame1.quat.w, sizeof(float));
//
//
//				frame1.scale.x = 1.0f;
//				frame1.scale.y = 1.0f;
//				frame1.scale.z = 1.0f;
//
//
//
//				frame1.quat.x = -frame1.quat.x;
//				frame1.quat.y = -frame1.quat.y;
//				frame1.quat.z = frame1.quat.z;
//				frame1.quat.w = frame1.quat.w;
//
//				keyframeArray.push_back(frame1);
//			}
//
//		}
//
//};
