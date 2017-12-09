#pragma once
#ifndef CORE_H
#define CORE_H
#endif



//CRUCIAL
#include <windows.h>
#include <DirectXMath.h>
#include <DirectXMathMatrix.inl>
#include <DirectXMathVector.inl>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <string>
#include <iostream>
#include <sstream>
#include <vector>
#include <memory>


//INCLUDES
#include "Timer.h"
#include "Input.h"
//#include "MeshObject.h"
#include "../DirectXTK/Inc/DDSTextureLoader.h"
#include "../DirectXTK/Inc/WICTextureLoader.h"
#include "..\Headers\Reader.h"
#include "BINimporter.h"

//VISUAL
#include "ShadowMapping.h"

//SIMPLIFIERS
#include "../DirectXTK/Inc/SimpleMath.h"
#include "../DirectXTK/Inc/SpriteFont.h"
#include "../DirectXTK/Inc/SpriteBatch.h"



using namespace DirectX;
using namespace DirectX::SimpleMath;



#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")


class Core{ //Abstract class


public:
	//Input handler for keyboard and mouse interactions
	Input* m_Input = new Input();
	IDirectInputDevice8* Keyboard;
	IDirectInputDevice8* Mouse;
	DIMOUSESTATE mouseLastState;
	LPDIRECTINPUT8 DirectInput;

	bool hit;

	int renderBool = 0;
	bool highttest = 0;
	float hTest = 0;
	float timeIterator = 2;
	int TheFps = 0;

	//FONTS AND BATCHES
	std::unique_ptr<DirectX::SpriteFont> spritefont;
	std::unique_ptr<DirectX::SpriteBatch> spriteBatch;

	std::wostringstream outs;
public:
	//Window Constructor
	Core(HINSTANCE hInstance, std::string name, UINT scrW, UINT scrH);
	virtual ~Core();

	//Core functions
	bool InitDirectInput(HINSTANCE hInstance);
	virtual bool Init();
	void SetViewport();
	int Run();
	void Update(float dt);
	void Render();

	void makelights();

	//Render functions
	virtual void Release();
	void InputHandler();
	void SetFonts();


	ID3D11BlendState*     AlphaToCoverageBS = nullptr;



	// MESH IMPORTER
	BINimporter theCustomImporter;
	vector<int> intArrayTex;

	//MESH ARRAY
	//	std::vector<MeshObject*> meshObjects;


	//Shadows
	ShadowMap* shadowMap;
	ID3D11ShaderResourceView* shadowTexture = nullptr;
	
	//Filemap reader
	Reader* fileMapReader;
	int fileMapSize = 50;

public:
	//void AddObject(MeshObject mObj);

	//Fps camera
	Vector4 camPosition;
	Vector4 camTarget;
	Vector4 camForward = Vector4(0.0f, 0.0f, -1.0f, 0.0f);
	Vector4 camUp = Vector4(0.0f, 1.0f, 0.0f, 0.0f);
	Vector4 camRight = Vector4(1.0f, 0.0f, 0.0f, 0.0f);

	Vector4 DefaultForward = Vector4(0.0f, -1.0f, 0.0f, 0.0f);
	Vector4 DefaultRight = Vector4(-1.0f, 0.0f, 0.0f, 0.0f);

	XMMATRIX camRotationMatrix;
	XMMATRIX fpsCamLook;
	XMMATRIX CamProjection;
	XMMATRIX CamView;

	float moveLeftRight = 0.0f;
	float moveBackForward = 0.0f;
	float camYaw = 0.0f;
	float camPitch = 0.0f;
	float speed = 0.0f;
	float boost = 0.0f;
	float zoom = 0.0f;

	Camera fpsCam;
	POINT mLastMousePos;

	//Fps counter calc func
	void fpscounter();
	Timer gTimer;

	//Window name
	std::wstring mainwname;

	//Geometry functions
	void CreatePlaneDataAndBuffers();
	void Shaders();
	void TextureFunc();


	HRESULT CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob);

	//Message handler
	LRESULT MsgProc(HWND hwindow, UINT msg, WPARAM wParam, LPARAM lParam);
	HRESULT KUK = 0;


	//Bool to test Backface culling
	bool Bculling = FALSE;
	bool Bcullingcheck = FALSE;

	////STRUCT DESCRIPTIONS FOR CBUFFERS

	//Deffered cBuffer
	struct World2
	{
		XMFLOAT4X4 WVP;
		XMFLOAT4X4 World;

	};

	//Structs for cBuffers
	struct World
	{
		XMFLOAT4X4 View;
		XMFLOAT4X4 Projection;
		XMFLOAT4X4 WorldSpace;
		XMFLOAT4X4 InvWorld;
		//XMFLOAT4X4 WVP;
		////Shadow matrixs
		//XMFLOAT4X4 lightView;
		//XMFLOAT4X4 lightProjection;
	};

	struct LightMatrix{
		//Shadow matrixs
		XMFLOAT4X4 lightView;
		XMFLOAT4X4 lightProjection;
	};
	//LightBuffers
	struct LightStruct{

		DirLight		lDir;
		PointLight    pLights[10];
		SpotLight spot;
		//float	pad;

	};


	float lightmove = 0;

	struct MatView{
		Material gMaterial;
		XMFLOAT3	gEyePos;
		int	hasTexture;
		//int		pad;

	};

	struct WorldWireFrame{
		XMFLOAT4X4 View;
		XMFLOAT4X4 Projection;
		XMFLOAT4X4 WorldSpace;
	};

	struct HeightmapInfo{
		float heightElements;
		XMFLOAT3 pad1;

		HeightmapInfo hmInfoConstant(){
			pad1.x = 1;
			pad1.y = 1;
			pad1.z = 1;
		}
	};

	//ViewPoint struct
	struct ViewBufferStruct{
		XMFLOAT3 viewPoint;
		float pad;
	};

	//STRUCTS FOR CBUFFERS
	WorldWireFrame WorldMatrixWF;
	MatView MatBuffer1;
	HeightmapInfo heightmapInfo;
	World2 WorldMatrix2;
	LightMatrix LightMatrix1;
	LightStruct PrimaryLights;
	World WorldMatrix1;
	ViewBufferStruct ViewP;

	size_t* test = 0;
	
	//VertexBuffer for picking, saves vertexpositions
	struct VertexPos{
		float vx, vy, vz;
	};


	struct Float3{
		float x, y, z;
		Float3(float x, float y, float z){
			this->x = x;
			this->y = y;
			this->z = z;
		}
	};

	//Window handlers
	HWND hWindow;
	HINSTANCE hInstance;

	//Start Window functions
	bool InitWindow();
	bool InitDirect3D(HWND);

public:

	//Core varibles for often use
	std::string applicationName;
	UINT screen_Width;
	UINT screen_Height;
	DWORD windowStyle; //se konstruktorn

	BoundingFrustum frustum;

	//VIEWPORTS
	D3D11_VIEWPORT vp;

	//MATRIX
	XMMATRIX identityM = XMMatrixIdentity();
	XMMATRIX WorldMatrix;
	//Core functions for windown and program setup
	ID3D11Device* gDevice = nullptr;
	ID3D11DeviceContext* gDeviceContext = nullptr;
	ID3D11RenderTargetView* gBackRufferRenderTargetView = nullptr;
	IDXGISwapChain* gSwapChain = nullptr;
	ID3D11DepthStencilView* gDepthStencilView = nullptr;
	//2D Textures
	ID3D11Texture2D* depthStencilBuffer = nullptr;

	//Buffers
	ID3D11Buffer* gVertexBuffer = nullptr;
	ID3D11Buffer* gVertexBuffer2 = nullptr;
	ID3D11Buffer* gWorld = nullptr;
	ID3D11Buffer* gWorld2 = nullptr;
	ID3D11Buffer* heightmapInfoConstant;
	ID3D11Buffer* ViewBuffer = nullptr;
	ID3D11Buffer* gLights = nullptr;
	ID3D11Buffer* PrimaryLightBuffer = nullptr;
	ID3D11Buffer* MatBuffer = nullptr;
	ID3D11Buffer* shadowBuffer = nullptr;
	ID3D11Buffer* gLMat = nullptr;

	//Shader resources for textures and more
	ID3D11ShaderResourceView* ddsTex1 = nullptr;
	ID3D11ShaderResourceView* ddsTex2 = nullptr;
	ID3D11ShaderResourceView* ddsTex3 = nullptr;
	ID3D11ShaderResourceView*   normalMap = nullptr;
	ID3D11ShaderResourceView* AdsResourceView = nullptr;
	ID3D11ShaderResourceView* NpcRV = nullptr;
	ID3D11ShaderResourceView** RSWArray = nullptr;

	//Vertex/geometry layout desc
	ID3D11InputLayout* gVertexLayout = nullptr;
	ID3D11InputLayout* gVertexLayout2 = nullptr;
	ID3D11InputLayout* gWireFrameLayout = nullptr;
	ID3D11InputLayout* gSplatmapLayout = nullptr;
	ID3D11InputLayout* animLayout = nullptr;
	ID3D11InputLayout* animLayout2;

	//GLOW BLOB THAT TAKES A TRIP TO GLOW.H
	ID3DBlob* glowBlob = nullptr;

	//Samplers
	ID3D11SamplerState* sampState1 = nullptr;
	ID3D11SamplerState* sampState2 = nullptr;
	ID3D11BlendState* gBlendStateTransparency = nullptr;
	ID3D11BlendState* gBlendStateDefault = nullptr;

	//Shaders
	ID3D11VertexShader* splatMapVertexShader = nullptr;
	ID3D11VertexShader* gVertexShader = nullptr;
	ID3D11VertexShader* dVertexShader = nullptr;
	ID3D11VertexShader* shadowVertexShader = nullptr;
	ID3D11VertexShader* gWireFrameVertexShader = nullptr;
	ID3D11VertexShader* shader2DVS = nullptr;
	ID3D11VertexShader* glowVertexShader = nullptr;
	ID3D11VertexShader* horizontalBlurVertexShader = nullptr;
	ID3D11VertexShader* verticalBlurVertexShader = nullptr;
	ID3D11VertexShader* animVertexShader = nullptr;
	ID3D11VertexShader* heightVertexShader = nullptr;


	ID3D11GeometryShader* gGeometryShader = nullptr;
	ID3D11GeometryShader* gBackFaceShader = nullptr;

	ID3D11PixelShader* gPixelShader = nullptr;
	ID3D11PixelShader* dPixelShader = nullptr;
	ID3D11PixelShader* splatMapPixelShader = nullptr;
	ID3D11PixelShader* hitPixelShader = nullptr;
	ID3D11PixelShader* shadowPixelShader = nullptr;
	ID3D11PixelShader* gWireFramePixelShader = nullptr;
	ID3D11PixelShader* gWireFramePixelShaderCONTAINTEST = nullptr;
	ID3D11PixelShader* shader2DPS = nullptr;
	ID3D11PixelShader* glowPixelShader = nullptr;
	ID3D11PixelShader* horizontalBlurPixelShader = nullptr;
	ID3D11PixelShader* verticalBlurPixelShader = nullptr;
	ID3D11PixelShader* animPixelShader = nullptr;
	ID3D11PixelShader* heightPixelShader = nullptr;

	// KEYBOARD AND MOUSE STUFF
	DIMOUSESTATE mouseCurrState;
	BYTE keyboardState[256];

	//Render States
	ID3D11RasterizerState* NoBcull;
	ID3D11Buffer* d2dIndexBuffer;
	int m_bitmapWidth, m_bitmapHeight = 100;

	//blendstates and transparency
	void BlendStates();
	ID3D11BlendState *transparency;
	ID3D11RasterizerState *counterCWCullmode; //dessa används vid transpareny mojs
	ID3D11RasterizerState *CWCullmode;

	//ENABLE/DISABLE DEPTH
	ID3D11DepthStencilState* m_depthDisabledStencilState;
	ID3D11DepthStencilState* m_depthStencilState;
	void TurnZBufferOn();
	void TurnZBufferOff();

	void AlphaBlendOn();
	void AlphaBlendOff();
	void setDefaultRender();
	// SHADER TESTER
	ID3D11Buffer* shaderTest = nullptr;

	//Structs for cBuffers
	struct Options
	{
		int option1;
		int option2;
		int option3;
		int option4;
		int option5;
		int option6;
		int option7;
		int option8;
	};
	Options optionStruct;

public:

	//Random values
	int speedMultiplier = 1;
	float fps2;

};