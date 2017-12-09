
#include "..\Headers\Core.h"
#include <string.h>



namespace{
	Core* pCore; //pekare till sjävla applikationen
}

// MSG HANDLER FOR WINMAIN

LRESULT CALLBACK MainWindowProc(HWND hwindow, UINT msg, WPARAM wParam, LPARAM lParam){ //denna används endast för att komma åt Corens MsgProc() och skicka vidare meddelanden till den
	return pCore->MsgProc(hwindow, msg, wParam, lParam);
}

// CONSTRUCTOR

Core::Core(HINSTANCE hInstance, std::string name, UINT scrW, UINT scrH){
	this->hInstance = hInstance;
	applicationName = name;
	screen_Width = scrW;
	screen_Height = scrH;
	pCore = this;
	windowStyle = WS_OVERLAPPED | WS_CAPTION | WS_MINIMIZEBOX;
}

// DESTRUCTOR

Core::~Core(){ //destruktor
	Core::Release();
}


std::wstring string2wString(const std::string& s){
	int len;
	int slength = (int)s.length() + 1;
	len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
	wchar_t* buf = new wchar_t[len];
	MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, buf, len);
	std::wstring r(buf);
	delete[] buf;
	return r;
}
// INITIALIZE CORE FUNCTIONS

bool Core::Init(){

	HRESULT inputtest;


	CoInitialize(nullptr);

	if (!InitWindow()){
		return false; //gick inte att skapa window
	}

	if (!InitDirect3D(hWindow)){
		return false; //gick inte att skapa Direct3D
	}
	// Sets and creates viewport
	SetViewport();
	//set values for 200 lights xD
	makelights();

	//LIGHT
	//Deafult Lights
	//Dir light1                    PLACE IN OWN FUNCTION LATER
	PrimaryLights.lDir.Ambient = XMFLOAT4(0.05f, 0.05f, 0.05f, 0.0f);
	PrimaryLights.lDir.Diffuse = XMFLOAT4(0.35f, 0.35f, 0.35f, 1.0f);
	PrimaryLights.lDir.Specular = XMFLOAT4(0.4f, 0.4f, 0.4f, 2.0f);
	PrimaryLights.lDir.Dir = XMFLOAT3(0.0f, -10.0f, 0.8f);
	PrimaryLights.lDir.Pad = 0.0f;
	////////////LIGHTS/////////////////////////////////////////////////////

	// Deafult Lights
	////Padding
//	MatBuffer1.pad = 0;
	//Eyepos
	MatBuffer1.gEyePos = Vector3(camPosition.x, camPosition.y, camPosition.z);
	// Makes a deafualt mat

	CreatePlaneDataAndBuffers();

	//Set Camera values
	camPosition = Vector4(50.0f, 10.70f, -50.0f, 0.0f);
	fpsCam.SetLens(0.25f*3.14f, float(screen_Width / screen_Height), 1.0f, 1000.0f);

	//Initialize Shaders and triangle data
	Shaders();
	BlendStates();

	TextureFunc();


	//FONTS
	spritefont.reset(new DirectX::SpriteFont(gDevice, L"Fonts/Arial.spritefont"));
	spriteBatch.reset(new DirectX::SpriteBatch(gDeviceContext));

	//FilemapReader
	fileMapReader = new Reader;
	//Create the consumer handle for the filemap (Shared memory)
	fileMapReader->Initizalize(size_t(fileMapSize*(1024*1024)), size_t(5), size_t(5));
	fileMapReader->createFileMap(fileMapReader->kMemSizeMegaBytes);
	// Set default render values
	setDefaultRender();

	// CREATE INPUT OBJECT
	InitDirectInput(hInstance);
	inputtest = m_Input->Initialize(hInstance, hWindow, screen_Width, screen_Height);
	if (inputtest == 0)
	{
		MessageBox(hWindow, L"Could not initialize the input object.", L"Error", MB_OK);
		return false;
	}

	return true; //om båda funkade så returnera true (y)

}

// INITIALIZE WINDOW

bool Core::InitWindow(){
	WNDCLASSEX wcex;
	ZeroMemory(&wcex, sizeof(WNDCLASSEX));
	wcex.hInstance = hInstance;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.cbSize = sizeof(WNDCLASSEX);
	wcex.style = CS_HREDRAW | CS_VREDRAW; // hur den ska måla ut allt, fast spelar nog ingen roll i vårt fall
	wcex.lpfnWndProc = MainWindowProc;
	//wcex.hCursor = LoadCursor(NULL, IDC_NO); //skoj
	//wcex.hIcon = LoadIcon(NULL, IDI_ERROR); //skoj * 2
	//wcex.hbrBackground = (HBRUSH)GetStockObject(NULL_BRUSH);
	wcex.lpszMenuName = NULL;
	wcex.lpszClassName = L"CoreCLASS";
	wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	if (!RegisterClassEx(&wcex)){
		MessageBox(NULL, L"Kunde inte registrera window classen", NULL, NULL);
		return false;
	}

	RECT r = { LONG(0.0f), LONG(0.0f), LONG(screen_Width), LONG(screen_Height) };
	AdjustWindowRect(&r, windowStyle, FALSE); //kommer inte kunna resiza skiten
	int width = r.right - r.left;
	int height = r.bottom - r.top;
	mainwname = L"SlothEngine 0.009";
	hWindow = CreateWindow(L"CoreCLASS",
		L"DX11 Editor ", //INTE SÄKER PÅ DETTA, namnet på applikationen blir typ i kinaspråk så venne, kan vara detta
		WS_OVERLAPPEDWINDOW,//Window handlers
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		width, height, nullptr, nullptr, hInstance, nullptr);

	if (!hWindow){
		MessageBox(NULL, L"Failed to create window", NULL, NULL);
		return false;
	}

	return true;
}

// CREATE VIEWPORT

void Core::SetViewport()
{
	vp.Width = (float)screen_Width;
	vp.Height = (float)screen_Height;
	vp.MinDepth = 0.0f;
	vp.MaxDepth = 1.0f;
	vp.TopLeftX = 0;
	vp.TopLeftY = 0;

}

// FPS COUNTER

void Core::fpscounter()
{
	// Code computes the avarage frame time and frames per second

	static int framecount = 0;
	static float time = 0.0f;

	//Function is beeing initialiased every time a frame is made
	framecount++;

	// Compute averages over one second period
	if ((gTimer.TotalTime() - time) >= 0.25f)
	{
		float fps = (float)framecount * 4; // fps = framecount / 1
		float mspf = 1000.0f / fps;
		float timer = gTimer.TotalTime();
		// Makes a String for the window handler

		fps2 = fps;


		outs.precision(6);
		outs << mainwname << L" "
			<< L"        FPS: " << fps << L"\n "
			<< L"        Frame Time: " << mspf << L" (ms)"
			<< L"        Time: " << timer << L" sec";

		//Prints the text in the window handler
			SetWindowText(hWindow, L"SlothEngine v0.021");// outs.str().c_str());

		// Reset for next fps.
		framecount = 0;
		time += 0.25f;
		timeIterator = time;
		TheFps = int(fps);
	}
}

// CREATE TEXTURES

void Core::TextureFunc(){

	HRESULT texCheck;
	texCheck = CreateWICTextureFromFile(gDevice, L"Textures//maleNormalmap2.jpg", nullptr, &normalMap);
	texCheck = CreateDDSTextureFromFile(gDevice, L"Textures/normalTest.jpg", nullptr, &ddsTex1);
	texCheck = CreateDDSTextureFromFile(gDevice, L"Textures/shark.dds", nullptr, &ddsTex2);
	texCheck = CreateDDSTextureFromFile(gDevice, L"Textures/happy-smug-sloth.dds", nullptr, &AdsResourceView);
	texCheck = CreateWICTextureFromFile(gDevice, L"Textures/errorstop_23.jpg", nullptr, &ddsTex3);


}

// COMPILE SHADERS FROM .HLSL FILES

HRESULT Core::CompileShader(_In_ LPCWSTR srcFile, _In_ LPCSTR entryPoint, _In_ LPCSTR profile, _Outptr_ ID3DBlob** blob)
{
	if (!srcFile || !entryPoint || !profile || !blob)
		return E_INVALIDARG;

	*blob = nullptr;

	UINT flags = D3DCOMPILE_ENABLE_STRICTNESS;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
#endif

	const D3D_SHADER_MACRO defines[] =
	{
		"EXAMPLE_DEFINE", "1",
		NULL, NULL
	};

	ID3DBlob* shaderBlob = nullptr;
	ID3DBlob* errorBlob = nullptr;
	HRESULT hr = D3DCompileFromFile(srcFile, defines, D3D_COMPILE_STANDARD_FILE_INCLUDE,
		entryPoint, profile,
		flags, 0, &shaderBlob, &errorBlob);
	if (FAILED(hr))
	{
		if (errorBlob)
		{
			OutputDebugStringA((char*)errorBlob->GetBufferPointer());
			errorBlob->Release();
		}

		if (shaderBlob)
			shaderBlob->Release();

		return hr;
	}

	*blob = shaderBlob;

	return hr;
}

// CREATE AND COMPILE SHADERS

void Core::Shaders(){

	HRESULT ShaderTest = 0;

	//MAKE SAMPLERS
	D3D11_SAMPLER_DESC samplerDesc;
	ZeroMemory(&samplerDesc, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MaxAnisotropy = 16;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
	gDevice->CreateSamplerState(&samplerDesc, &sampState1);

	D3D11_SAMPLER_DESC samplerDesc2;
	ZeroMemory(&samplerDesc2, sizeof(D3D11_SAMPLER_DESC));
	samplerDesc2.Filter = D3D11_FILTER_ANISOTROPIC;
	samplerDesc2.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc2.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc2.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	samplerDesc2.MaxAnisotropy = 16;
	gDevice->CreateSamplerState(&samplerDesc2, &sampState2);

	D3D11_RENDER_TARGET_BLEND_DESC blendStateRTDesc;
	blendStateRTDesc.BlendEnable = true;
	blendStateRTDesc.SrcBlend = D3D11_BLEND_ONE;
	blendStateRTDesc.DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
	blendStateRTDesc.BlendOp = D3D11_BLEND_OP_ADD;
	blendStateRTDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	blendStateRTDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	blendStateRTDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendStateRTDesc.RenderTargetWriteMask = 0x0f;

	D3D11_BLEND_DESC blendStateDesc;
	ZeroMemory(&blendStateDesc, sizeof(D3D11_BLEND_DESC));
	blendStateDesc.AlphaToCoverageEnable = false;
	blendStateDesc.RenderTarget[0] = blendStateRTDesc;

	ShaderTest = gDevice->CreateBlendState(&blendStateDesc, &gBlendStateTransparency);

	//DEAFULT SHADERS
	ID3DBlob* layoutblobl = nullptr;

	//create vertex shaders
	ID3DBlob* pVS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/deafultVS.hlsl", "VS_main", "vs_5_0", &pVS);
	KUK = gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &gVertexShader);

	//create vertex depth
	ID3DBlob* dVS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/depthVS.hlsl", "VS_main", "vs_5_0", &dVS);
	ShaderTest = gDevice->CreateVertexShader(dVS->GetBufferPointer(), dVS->GetBufferSize(), nullptr, &dVertexShader);

	//create vertex texture
	ID3DBlob* deafaultVS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/deafultTexVS.hlsl", "VS_main", "vs_5_0", &deafaultVS);
	ShaderTest = gDevice->CreateVertexShader(deafaultVS->GetBufferPointer(), deafaultVS->GetBufferSize(), nullptr, &shader2DVS);

	ID3DBlob* dVS2 = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/RGBSplatmapVS.hlsl", "main", "vs_5_0", &dVS2);
	ShaderTest = gDevice->CreateVertexShader(dVS2->GetBufferPointer(), dVS2->GetBufferSize(), nullptr, &splatMapVertexShader);

	//GLOW SHADERS
	ShaderTest = CompileShader(L"FX_HLSL/GlowVS.hlsl", "VS_main", "vs_5_0", &glowBlob);
	ShaderTest = gDevice->CreateVertexShader(glowBlob->GetBufferPointer(), glowBlob->GetBufferSize(), nullptr, &glowVertexShader);


	ID3DBlob* pSVS3 = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/HorizontalBlurVS.hlsl", "main", "vs_5_0", &pSVS3);
	ShaderTest = gDevice->CreateVertexShader(pSVS3->GetBufferPointer(), pSVS3->GetBufferSize(), nullptr, &horizontalBlurVertexShader);

	ID3DBlob* pSVS4 = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/VerticalBlurVS.hlsl", "main", "vs_5_0", &pSVS4);
	ShaderTest = gDevice->CreateVertexShader(pSVS4->GetBufferPointer(), pSVS4->GetBufferSize(), nullptr, &verticalBlurVertexShader);

	//Create shadow vertexShader
	ID3DBlob* pSVS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/shadowVS.hlsl", "VS_main", "vs_5_0", &pSVS);
	ShaderTest = gDevice->CreateVertexShader(pSVS->GetBufferPointer(), pSVS->GetBufferSize(), nullptr, &shadowVertexShader);


	ID3DBlob* pHVS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/heightMapVertexShader.hlsl", "VS_main", "vs_5_0", &pHVS);
	ShaderTest = gDevice->CreateVertexShader(pHVS->GetBufferPointer(), pHVS->GetBufferSize(), nullptr, &heightVertexShader);


	ID3DBlob* aVS2 = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/animVS.hlsl", "VS_main", "vs_5_0", &aVS2);
	ShaderTest = gDevice->CreateVertexShader(aVS2->GetBufferPointer(), aVS2->GetBufferSize(), nullptr, &animVertexShader);


	// INPUT LAYOUT MÅSTE VARA ANPASSAD TILL VERTEX SHADER
	//create input layout (verified using vertex shader)
	D3D11_INPUT_ELEMENT_DESC inputDescAnim[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 2, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 3, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "BLENDWEIGHT", 4, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		//{ "BLENDINDICES", 5, DXGI_FORMAT_R8G8B8A8_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D11_INPUT_ELEMENT_DESC inputDesc3[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "PADDING", 0, DXGI_FORMAT_R32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};
	D3D11_INPUT_ELEMENT_DESC inputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }

	};
	//create input layout (verified using vertex shader)
	D3D11_INPUT_ELEMENT_DESC inputDesc2[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 20, D3D11_INPUT_PER_VERTEX_DATA, 0 }

	};
		//create input layout (verified using vertex shader)
	D3D11_INPUT_ELEMENT_DESC DefaultInputDesc[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 }

	};

	D3D11_INPUT_ELEMENT_DESC inputDescSplatmap[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "TEXCOORDALPHA", 0, DXGI_FORMAT_R32G32_FLOAT, 20, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 28, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	D3D11_INPUT_ELEMENT_DESC inputDescPosOnly[] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
	};

	//CREATE  THE LAYOUTS
	ShaderTest = gDevice->CreateInputLayout(inputDescSplatmap, ARRAYSIZE(inputDescSplatmap), pVS->GetBufferPointer(), pVS->GetBufferSize(), &gSplatmapLayout);
	ShaderTest = gDevice->CreateInputLayout(DefaultInputDesc, ARRAYSIZE(DefaultInputDesc), deafaultVS->GetBufferPointer(), deafaultVS->GetBufferSize(), &gVertexLayout);

	ShaderTest = gDevice->CreateInputLayout(inputDesc3, ARRAYSIZE(inputDesc3), aVS2->GetBufferPointer(), aVS2->GetBufferSize(), &animLayout2);

	//Create geometry shader
	ID3DBlob* gGS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/BackFaceGShader.hlsl", "gs_main", "gs_5_0", &gGS);
	ShaderTest = gDevice->CreateGeometryShader(gGS->GetBufferPointer(), gGS->GetBufferSize(), nullptr, &gBackFaceShader);



	//create pixel shader
	ID3DBlob* pPS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/deafultPS.hlsl", "PS_main", "ps_5_0", &pPS);
	ShaderTest = gDevice->CreatePixelShader(pPS->GetBufferPointer(), pPS->GetBufferSize(), nullptr, &gPixelShader);

	//create hitpixel shader
	ID3DBlob* pPSH = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/hitPixelShader.hlsl", "PS_main", "ps_5_0", &pPSH);
	ShaderTest = gDevice->CreatePixelShader(pPSH->GetBufferPointer(), pPSH->GetBufferSize(), nullptr, &hitPixelShader);

	//create depth pixelshader
	ID3DBlob* dPS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/depthPS.hlsl", "PS_main", "ps_5_0", &dPS);
	ShaderTest = gDevice->CreatePixelShader(dPS->GetBufferPointer(), dPS->GetBufferSize(), nullptr, &dPixelShader);

	//create pixel texture
	ID3DBlob* ddPS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/defaultTexPS.hlsl", "PS_main", "ps_5_0", &ddPS);
	ShaderTest = gDevice->CreatePixelShader(ddPS->GetBufferPointer(), ddPS->GetBufferSize(), nullptr, &shader2DPS);


	////create height pixel shader
	ID3DBlob* pPS2 = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/RGBSplatmapPS.hlsl", "main", "ps_5_0", &pPS2);
	ShaderTest = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &splatMapPixelShader);

	////create shadow pixel shader
	ID3DBlob* pSPS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/shadowPS.hlsl", "PS_main", "ps_5_0", &pSPS);
	ShaderTest = gDevice->CreatePixelShader(pSPS->GetBufferPointer(), pSPS->GetBufferSize(), nullptr, &shadowPixelShader);

	////create Height pixel shader
	ID3DBlob* pHPS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/heightMapPixelShader.hlsl", "PS_main", "ps_5_0", &pHPS);
	ShaderTest = gDevice->CreatePixelShader(pHPS->GetBufferPointer(), pHPS->GetBufferSize(), nullptr, &heightPixelShader);

	////create Animatiob pixel shader
	ID3DBlob* aPS = nullptr;
	ShaderTest = CompileShader(L"FX_HLSL/animPS.hlsl", "PS_main", "ps_5_0", &aPS);
	ShaderTest = gDevice->CreatePixelShader(aPS->GetBufferPointer(), aPS->GetBufferSize(), nullptr, &animPixelShader);


	//WIREFRAME	
	ShaderTest = CompileShader(L"FX_HLSL/WireFrameVS.hlsl", "main", "vs_5_0", &pVS);
	ShaderTest = gDevice->CreateVertexShader(pVS->GetBufferPointer(), pVS->GetBufferSize(), nullptr, &gWireFrameVertexShader);

	ShaderTest = CompileShader(L"FX_HLSL/WireFramePS.hlsl", "main", "ps_5_0", &pPS2);
	ShaderTest = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &gWireFramePixelShader);

	ShaderTest = CompileShader(L"FX_HLSL/WireFramePSContainsTest.hlsl", "main", "ps_5_0", &pPS2);
	ShaderTest = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &gWireFramePixelShaderCONTAINTEST);

	ShaderTest = CompileShader(L"FX_HLSL/GlowPS.hlsl", "main", "ps_5_0", &pPS2);
	ShaderTest = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &glowPixelShader);

	ShaderTest = CompileShader(L"FX_HLSL/HorizontalBlurPS.hlsl", "main", "ps_5_0", &pPS2);
	ShaderTest = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &horizontalBlurPixelShader);

	ShaderTest = CompileShader(L"FX_HLSL/VerticalBlurPS.hlsl", "main", "ps_5_0", &pPS2);
	ShaderTest = gDevice->CreatePixelShader(pPS2->GetBufferPointer(), pPS2->GetBufferSize(), nullptr, &verticalBlurPixelShader);

	gGS->Release();
	pPSH->Release();
	pPS->Release();
	pPS2->Release();
	pSPS->Release();
	pSVS->Release();
	ddPS->Release();
	deafaultVS->Release();
	pSVS3->Release();

}

// CREATE BUFFERS AND TEMP PLANE

void Core::CreatePlaneDataAndBuffers(){
	HRESULT BufferTest;

	struct PlaneVertex
	{
		float x, y, z;
		float ux, uy;
		float nx, ny, nz;
		float nx2, ny2, nz2;
	}
	PlaneVertices[4] =
	{
		-1.0f, -1.0f, -1.0f,		//v0 
		0.0f, 1.0f,			//t0
		0.0f, 1.0f, 1.0f,		//		//n3
		0.0f, 1.0f, 1.0f,


		-1.0f, 1.0f, -1.0f,		//v1
		0.0f, 0.0f,				//t1
		0.0f, 1.0f, 1.0f,			//n3
		0.0f, 1.0f, 1.0f,

		1.0f, -1.0f, -1.0f,		//v2
		1.0f, 1.0f,			//t2
		0.0f, 1.0f, 1.0f,			//n3
		0.0f, 1.0f, 1.0f,

		1.0f, 1.0f, -1.0f,		//v3
		1.0f, 0.0f,			//t3
		0.0f, 1.0f, 1.0f,
		1.0f, 1.0f, -1.0f,		//v3

	};


	// VertexBuffer description
	D3D11_BUFFER_DESC bufferDesc;
	memset(&bufferDesc, 0, sizeof(bufferDesc));
	bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bufferDesc.Usage = D3D11_USAGE_DEFAULT;
	bufferDesc.ByteWidth = sizeof(PlaneVertices);

	D3D11_SUBRESOURCE_DATA data;
	data.pSysMem = PlaneVertices;
	gDevice->CreateBuffer(&bufferDesc, &data, &gVertexBuffer);

	// Rotatation And transform World Buffer
	D3D11_BUFFER_DESC transformbuffer;
	memset(&transformbuffer, 0, sizeof(transformbuffer));
	transformbuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	transformbuffer.Usage = D3D11_USAGE_DEFAULT;
	transformbuffer.ByteWidth = sizeof(World);
	BufferTest = gDevice->CreateBuffer(&transformbuffer, NULL, &gWorld);

	D3D11_BUFFER_DESC transformbuffer2;
	memset(&transformbuffer2, 0, sizeof(transformbuffer2));
	transformbuffer2.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	transformbuffer2.Usage = D3D11_USAGE_DEFAULT;
	transformbuffer2.ByteWidth = sizeof(World2);
	BufferTest = gDevice->CreateBuffer(&transformbuffer2, NULL, &gWorld2);

	// Rotatation And transform World Buffer
	D3D11_BUFFER_DESC lightMatrixBDesc;
	memset(&lightMatrixBDesc, 0, sizeof(lightMatrixBDesc));
	lightMatrixBDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightMatrixBDesc.Usage = D3D11_USAGE_DEFAULT;
	lightMatrixBDesc.ByteWidth = sizeof(LightMatrix);
	BufferTest = gDevice->CreateBuffer(&lightMatrixBDesc, NULL, &gLMat);


	//heightmap stuff, till splatmap
	D3D11_BUFFER_DESC heightmapBuffer;
	memset(&heightmapBuffer, 0, sizeof(heightmapBuffer));
	heightmapBuffer.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	heightmapBuffer.Usage = D3D11_USAGE_DEFAULT;
	heightmapBuffer.ByteWidth = sizeof(heightmapInfo);
	BufferTest = gDevice->CreateBuffer(&heightmapBuffer, NULL, &heightmapInfoConstant);

	// Lightbuffer
	D3D11_BUFFER_DESC lightbufferDesc;
	memset(&lightbufferDesc, 0, sizeof(lightbufferDesc));
	lightbufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightbufferDesc.Usage = D3D11_USAGE_DEFAULT;
	lightbufferDesc.CPUAccessFlags = 0;
	lightbufferDesc.MiscFlags = 0;
	lightbufferDesc.ByteWidth = sizeof(LightStruct);
	int bWidht = lightbufferDesc.ByteWidth;
	BufferTest = gDevice->CreateBuffer(&lightbufferDesc, NULL, &PrimaryLightBuffer);

	// ViewPoint Buffer
	D3D11_BUFFER_DESC ViewBufferDesc;
	ViewBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	memset(&ViewBufferDesc, 0, sizeof(ViewBufferDesc));
	ViewBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	ViewBufferDesc.CPUAccessFlags = 0;
	ViewBufferDesc.MiscFlags = 0;
	ViewBufferDesc.ByteWidth = sizeof(ViewBufferStruct);
	BufferTest = gDevice->CreateBuffer(&ViewBufferDesc, NULL, &ViewBuffer);

	// Mat Buffer
	D3D11_BUFFER_DESC MatBufferDesc;
	MatBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	memset(&MatBufferDesc, 0, sizeof(MatBufferDesc));
	MatBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	MatBufferDesc.CPUAccessFlags = 0;
	MatBufferDesc.MiscFlags = 0;
	MatBufferDesc.ByteWidth = sizeof(MatView);
	BufferTest = gDevice->CreateBuffer(&MatBufferDesc, NULL, &MatBuffer);

	//Options buffer
	D3D11_BUFFER_DESC OBufferDesc;
	OBufferDesc.Usage = D3D11_USAGE_DEFAULT;
	memset(&OBufferDesc, 0, sizeof(OBufferDesc));
	OBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	OBufferDesc.CPUAccessFlags = 0;
	OBufferDesc.MiscFlags = 0;
	OBufferDesc.ByteWidth = sizeof(Options);
	BufferTest = gDevice->CreateBuffer(&OBufferDesc, NULL, &shaderTest);



	// No backface culling description
	D3D11_RASTERIZER_DESC rastDesc1;
	ZeroMemory(&rastDesc1, sizeof(D3D11_RASTERIZER_DESC));
	rastDesc1.FillMode = D3D11_FILL_SOLID;
	rastDesc1.CullMode = D3D11_CULL_NONE;
	BufferTest = gDevice->CreateRasterizerState(&rastDesc1, &NoBcull);
}

// INITIALIZE DIRECTX OBJECT

bool Core::InitDirect3D(HWND hWindow){

	DXGI_SWAP_CHAIN_DESC scd;
	//Describe our SwapChain Buffer
	DXGI_MODE_DESC bufferDesc;

	ZeroMemory(&bufferDesc, sizeof(DXGI_MODE_DESC));

	bufferDesc.Width = screen_Width;
	bufferDesc.Height = screen_Height;
	bufferDesc.RefreshRate.Numerator = 144;
	bufferDesc.RefreshRate.Denominator = 1;
	bufferDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	bufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	bufferDesc.Scaling = DXGI_MODE_SCALING_CENTERED;

	ZeroMemory(&scd, sizeof(DXGI_SWAP_CHAIN_DESC));

	scd.BufferDesc = bufferDesc;
	scd.BufferCount = 1;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.OutputWindow = hWindow;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.SampleDesc.Count = 1; //anti-aliasing
	scd.SampleDesc.Quality = 0;// -kan vi mecka senare men är lite saker som ska göras då
	scd.Windowed = TRUE;
	scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
	scd.Flags = 0;

	// create a device, device context and swap chain using the information in the scd struct
	HRESULT deviceHr = D3D11CreateDeviceAndSwapChain(NULL,
		D3D_DRIVER_TYPE_HARDWARE,
		NULL,
		D3D11_CREATE_DEVICE_BGRA_SUPPORT,
		NULL,
		NULL,
		D3D11_SDK_VERSION,
		&scd,
		&gSwapChain,
		&gDevice,
		NULL,
		&gDeviceContext);

	if (SUCCEEDED(deviceHr))
	{
		// get the address of the back buffer
		ID3D11Texture2D* pBackBuffer = nullptr;
		gSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);

		// use the back buffer address to create the render target
		gDevice->CreateRenderTargetView(pBackBuffer, NULL, &gBackRufferRenderTargetView);
		pBackBuffer->Release();

		//DepthBuffer
		D3D11_TEXTURE2D_DESC depthStencilDesc;
		ZeroMemory(&depthStencilDesc, sizeof(D3D11_TEXTURE2D_DESC));
		depthStencilDesc.Width = screen_Width;
		depthStencilDesc.Height = screen_Height;
		depthStencilDesc.MipLevels = 1;
		depthStencilDesc.ArraySize = 1;
		depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
		depthStencilDesc.SampleDesc.Count = 1;
		depthStencilDesc.SampleDesc.Quality = 0;
		depthStencilDesc.Usage = D3D11_USAGE_DEFAULT;
		depthStencilDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
		depthStencilDesc.CPUAccessFlags = 0;
		depthStencilDesc.MiscFlags = 0;

		deviceHr = gDevice->CreateTexture2D(&depthStencilDesc, NULL, &depthStencilBuffer);
		deviceHr = gDevice->CreateDepthStencilView(depthStencilBuffer, NULL, &gDepthStencilView);

		D3D11_DEPTH_STENCIL_DESC depthStencilDesc2;
		D3D11_DEPTH_STENCIL_DESC depthDisabledStencilDesc;

		// Initialize the description of the stencil state.
		ZeroMemory(&depthStencilDesc2, sizeof(depthStencilDesc2));

		// Set up the description of the stencil state.
		depthStencilDesc2.DepthEnable = true;
		depthStencilDesc2.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthStencilDesc2.DepthFunc = D3D11_COMPARISON_LESS;

		depthStencilDesc2.StencilEnable = true;
		depthStencilDesc2.StencilReadMask = 0xFF;
		depthStencilDesc2.StencilWriteMask = 0xFF;

		// Stencil operations if pixel is front-facing.
		depthStencilDesc2.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc2.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthStencilDesc2.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc2.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Stencil operations if pixel is back-facing.
		depthStencilDesc2.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc2.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthStencilDesc2.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthStencilDesc2.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		HRESULT state;
		// Create the depth stencil state.
		state = gDevice->CreateDepthStencilState(&depthStencilDesc2, &m_depthStencilState);
		// Set the depth stencil state.
		gDeviceContext->OMSetDepthStencilState(m_depthStencilState, 1);

		// Clear the second depth stencil state before setting the parameters.
		ZeroMemory(&depthDisabledStencilDesc, sizeof(depthDisabledStencilDesc));

		// Now create a second depth stencil state which turns off the Z buffer for 2D rendering.  The only difference is 
		// that DepthEnable is set to false, all other parameters are the same as the other depth stencil state.
		depthDisabledStencilDesc.DepthEnable = false;
		depthDisabledStencilDesc.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;
		depthDisabledStencilDesc.DepthFunc = D3D11_COMPARISON_LESS;
		depthDisabledStencilDesc.StencilEnable = true;
		depthDisabledStencilDesc.StencilReadMask = 0xFF;
		depthDisabledStencilDesc.StencilWriteMask = 0xFF;
		depthDisabledStencilDesc.FrontFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.FrontFace.StencilDepthFailOp = D3D11_STENCIL_OP_INCR;
		depthDisabledStencilDesc.FrontFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.FrontFace.StencilFunc = D3D11_COMPARISON_ALWAYS;
		depthDisabledStencilDesc.BackFace.StencilFailOp = D3D11_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.BackFace.StencilDepthFailOp = D3D11_STENCIL_OP_DECR;
		depthDisabledStencilDesc.BackFace.StencilPassOp = D3D11_STENCIL_OP_KEEP;
		depthDisabledStencilDesc.BackFace.StencilFunc = D3D11_COMPARISON_ALWAYS;

		// Create the state using the device.
		state = gDevice->CreateDepthStencilState(&depthDisabledStencilDesc, &m_depthDisabledStencilState);


		// set the render target as the back buffer
		gDeviceContext->OMSetRenderTargets(1, &gBackRufferRenderTargetView, gDepthStencilView);

		return true; //returnerar att den HAR klarat av att skapa device och swapchain
	}

	return false; //det gick inte att skapa device och swapchain, snyft :'(
}

// MESSAGE HANDLER

LRESULT Core::MsgProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam){
	switch (msg){
	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_CHAR: //en tangentbordsknapp har tryckts i
		switch (wParam){
		case VK_ESCAPE:
			PostQuitMessage(0);
			return 0;
		}
	}

	return DefWindowProc(hwnd, msg, wParam, lParam);
}

// RUN PROGRAM

int Core::Run(){

	//om allt funkat:
	ShowWindow(hWindow, SW_SHOW);
	MSG msg = { 0 }; //töm alla platser i msg
	gTimer.Reset();

	// BoundingFrustum b(fpsCam.Proj());//frustum mot quadtree
	//frustum = b;
	//frustum.CreateFromMatrix(frustum, fpsCam.Proj());

	while (msg.message != WM_QUIT){
		if (PeekMessage(&msg, NULL, NULL, NULL, PM_REMOVE)){
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else{ //applikationen är fortfarande igång
			gTimer.Tick();
			Update(0.0f);
			//InputHandler();
			Render();
			fpscounter();
		}
	}
	return static_cast<int>(msg.wParam);
}

// RENDER

void Core::Render(){

	// Consume messages from maya
	fileMapReader->Consumer(gDevice);


	for (int i = 0; i < fileMapReader->meshes.size(); i++)
	{
			
		for (int u = 0; u < fileMapReader->materialsWithTextures.size(); u++)
		{
			if (fileMapReader->meshes[i].materialName == fileMapReader->materialsWithTextures[u])
			{
					fileMapReader->meshes[i].shaderResource1 = fileMapReader->materialsWithTextureSR[u];
					fileMapReader->meshes[i].hasTexture = TRUE;
			
			}
		}
		

	}


	//Update camera with values from maya
	fpsCam.mPosition = fileMapReader->tempCam.mPosition;
	fpsCam.mLook = fileMapReader->tempCam.mLook;
	fpsCam.mUp = fileMapReader->tempCam.mUp;
	fpsCam.mRight = fileMapReader->tempCam.mRight;
	
	fpsCam.View() = XMMatrixMultiply(XMMatrixScaling(1.0f, 1.0f, -1.0f),fpsCam.View());
	fpsCam.mIsOrtho = fileMapReader->tempCam.mIsOrtho;
	fpsCam.mOrthoWidthHeight = fileMapReader->tempCam.mOrthoWidthHeight;

	

	if (fpsCam.mIsOrtho == true)	
	{
	
		fpsCam.SetOrthoLens(fpsCam.mOrthoWidthHeight, fpsCam.mOrthoWidthHeight, 0.01f, 10000.0f);
 		fpsCam.mProj = fileMapReader->tempCam.mProj;
		fpsCam.alreadyIsOrtho = true;
		fpsCam.mPosition = fileMapReader->tempCam.mPosition;
		fpsCam.mLook = fileMapReader->tempCam.mLook;
		fpsCam.mUp = fileMapReader->tempCam.mUp;
		fpsCam.mRight = fileMapReader->tempCam.mRight;

		fpsCam.View() = XMMatrixMultiply(XMMatrixScaling(1.0f, 1.0f, -1.0f), fpsCam.View());
	}
	else if (fpsCam.mIsOrtho == false && fpsCam.alreadyIsOrtho == true)	//Extra bool added so that it wont set perspective lens each frame unnecessarily. Doesn't work on Ortho since W/H needs to be updated
	{
		fpsCam.SetLens(0.25f*3.14f, float(screen_Width / screen_Height), 1.0f, 1000.0f);
	}

	//Update cam
	fpsCam.UpdateViewMatrix();
	//fpsCam.mLook = XMFLOAT3(0, 0, 1);


	//Set vertex shader viewpoint buffer  (NOT USED IN DEFAULT SHADER)
	//gDeviceContext->UpdateSubresource(ViewBuffer, 0, NULL, &ViewP, 0, 0);
	//gDeviceContext->VSSetConstantBuffers(4, 1, &ViewBuffer);

	//Aquaire devices
	Keyboard->Acquire();
	Mouse->Acquire();

	//Get States from mouse and keyboard
	Mouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);
	Keyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);

	////////////LIGHTS/////////////////////////////////////////////////////



	//Update viewpoint buffer
	Vector3 viewPoint = Vector3(	fpsCam.mPosition.x, 	fpsCam.mPosition.y, 	fpsCam.mPosition.z);
	ViewP.viewPoint = viewPoint;
	ViewP.pad = 0;

	//// Update lightbuffer
	PrimaryLights.spot.Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1);
	PrimaryLights.spot.Diffuse = XMFLOAT4(0.1f, 0.05f, 0.05f, 1);
	PrimaryLights.spot.Specular = XMFLOAT4(0.2f, 0.2f, 0.2f, 1);
	PrimaryLights.spot.Pos = Vector3(fpsCam.mPosition.x, fpsCam.mPosition.y, fpsCam.mPosition.z);
	PrimaryLights.spot.Dir = XMFLOAT3(fpsCam.mLook.x - fpsCam.mPosition.x, fpsCam.mLook.y - fpsCam.mPosition.y, fpsCam.mLook.z - fpsCam.mPosition.z);
	PrimaryLights.spot.Pad = 1.0f;
	PrimaryLights.spot.Range = 10.0f;
	PrimaryLights.spot.Spot = 60.0f;
	PrimaryLights.spot.Attenuation = Vector3(0.00018f, 0.00018f, 0.00018f);
	MatBuffer1.gEyePos = Vector3(fpsCam.mPosition.x, fpsCam.mPosition.y, fpsCam.mPosition.z);
	//MatBuffer1.pad = 0;
	gDeviceContext->UpdateSubresource(PrimaryLightBuffer, 0, NULL, &PrimaryLights, 0, 0);
	gDeviceContext->PSSetConstantBuffers(0, 1, &PrimaryLightBuffer);

	//Set vertex shader view buffer
	gDeviceContext->UpdateSubresource(ViewBuffer, 0, NULL, &ViewP, 0, 0);
	gDeviceContext->VSSetConstantBuffers(4, 1, &ViewBuffer);
	////////////LIGHTS/////////////////////////////////////////////////////

	//Turn Z (Depth) buffer on
	

		UINT32 vertexSize = sizeof(float)* 8;
		UINT32 offset = 0;
		float clearColor[] = { 0.13f, 0.00f, 0.00f, 1.0f };
		gDeviceContext->ClearRenderTargetView(gBackRufferRenderTargetView, clearColor);
		gDeviceContext->ClearDepthStencilView(gDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
		setDefaultRender();
		for (int i = 0; i < fileMapReader->meshes.size(); i++) {

			gDeviceContext->IASetVertexBuffers(0, 1, &fileMapReader->meshes[i].vertexBuffer, &vertexSize, &offset);
			
			WorldMatrix = XMMatrixScaling(float(fileMapReader->meshes[i].mayaScale.x), float(fileMapReader->meshes[i].mayaScale.y), float(fileMapReader->meshes[i].mayaScale.z)) *XMMatrixRotationQuaternion(fileMapReader->meshes[i].mayaRot) *
				XMMatrixTranslation(float(fileMapReader->meshes[i].mayaPos.x), float(fileMapReader->meshes[i].mayaPos.y), float(fileMapReader->meshes[i].mayaPos.z));
			
	
			//View , projection and worldmatrixes sent to Vertex shader at slot 0
			XMStoreFloat4x4(&WorldMatrix1.View, XMMatrixTranspose(fpsCam.View()));
			XMStoreFloat4x4(&WorldMatrix1.Projection, XMMatrixTranspose(fpsCam.Proj()));
			XMStoreFloat4x4(&WorldMatrix1.WorldSpace, XMMatrixTranspose(WorldMatrix));
			XMStoreFloat4x4(&WorldMatrix1.InvWorld, XMMatrixTranspose(XMMatrixInverse(NULL, WorldMatrix)));
			gDeviceContext->UpdateSubresource(gWorld, 0, NULL, &WorldMatrix1, 0, 0);
			gDeviceContext->VSSetConstantBuffers(0, 1, &gWorld);
	

		
		
		//// Update matbuffer
		if (fileMapReader->meshes[i].hasMaterial == true)
		{
			MatBuffer1.gMaterial.Ambient = fileMapReader->meshes[i].material.Ambient;
			MatBuffer1.gMaterial.Diffuse = fileMapReader->meshes[i].material.Diffuse; 
			MatBuffer1.gMaterial.Specular = fileMapReader->meshes[i].material.Specular;
			MatBuffer1.gMaterial.Reflection = XMFLOAT4(0.2f, 0.2f, 0.2f, 1.0f);
			MatBuffer1.gMaterial.Specular.w = fileMapReader->meshes[i].material.SpecPow;
			MatBuffer1.hasTexture = fileMapReader->meshes[i].hasTexture;
			if (fileMapReader->meshes[i].hasTexture == 1)
			{ 
			gDeviceContext->PSSetShaderResources(0, 1, &fileMapReader->meshes[i].shaderResource1);
			MatBuffer1.gMaterial.Diffuse = Vector4(0.6f, 0.6f, 0.6f, 0.6f);
			}
			gDeviceContext->UpdateSubresource(MatBuffer, 0, NULL, &MatBuffer1, 0, 0);
			gDeviceContext->PSSetConstantBuffers(1, 1, &MatBuffer);
		}
		
		if(fileMapReader->meshes[i].faceCount>0)
		gDeviceContext->Draw(fileMapReader->meshes[i].faceCount*3,0);

		}

	//SetFonts();
	//gDeviceContext->VSSetShader(nullptr, nullptr, 0);
	//gDeviceContext->GSSetShader(nullptr, nullptr, 0);
	//gDeviceContext->PSSetShader(nullptr, nullptr, 0);
	//växla back/front buffer
	gSwapChain->Present(0, 0);	
}

// SET DEFAULT RENDER

void Core::setDefaultRender()
{

	//Set no backface culling to NULL and then set to a custom defined no backface culling state
	gDeviceContext->RSSetState(NULL);
	gDeviceContext->RSSetState(NoBcull);

	//NORMAL RENDER PASS FROM EYE POS 
	gDeviceContext->OMSetRenderTargets(1, &gBackRufferRenderTargetView, gDepthStencilView);
	gDeviceContext->RSSetViewports(1, &vp);

	// Set vertex layout and type of topology
	gDeviceContext->IASetInputLayout(gVertexLayout);
	gDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Set the shaders VS,GS,PS
	gDeviceContext->VSSetShader(shader2DVS, nullptr, 0);
	gDeviceContext->GSSetShader(nullptr, nullptr, 0);
	gDeviceContext->PSSetShader(shader2DPS, nullptr, 0);
	// Set the sampler states to the pixel shader.
	gDeviceContext->PSSetSamplers(0, 1, &sampState1);


}

// UPDATES

void Core::Update(float dt){


}

//REALESE AND CLEANUP

void Core::Release(){

	shadowMap->~ShadowMap();
	depthStencilBuffer->Release();
	m_Input->Shutdown();
	delete m_Input;
	m_Input = 0;

	//SHADER REALESE
	gVertexShader->Release();
	gPixelShader->Release();
	splatMapVertexShader->Release();
	shader2DVS->Release();
	shader2DVS->Release();
	dVertexShader->Release();
	dPixelShader->Release();
	shadowVertexShader->Release();
	shadowPixelShader->Release();
	gWireFramePixelShader->Release();
	glowPixelShader->Release();
	horizontalBlurPixelShader->Release();
	horizontalBlurVertexShader->Release();
	verticalBlurPixelShader->Release();
	verticalBlurVertexShader->Release();
	heightPixelShader->Release();

	//Sample realese
	sampState1->Release();
	sampState2->Release();

	//BlendState realese
	gBlendStateDefault->Release();
	gBlendStateTransparency->Release();

	//Layout release
	gVertexLayout->Release();
	gVertexLayout2->Release();
	gSplatmapLayout->Release();
	gWireFrameLayout->Release();

	NoBcull->Release();
	gVertexBuffer->Release();
	PrimaryLightBuffer->Release();

	//for (auto it = meshObjects.begin(); it != meshObjects.end(); it++){
	//	delete *it;
	//}
	//meshObjects.clear();

	counterCWCullmode->Release();
	transparency->Release();
	gDevice->Release();
	gDeviceContext->Release();
	gBackRufferRenderTargetView->Release();
	Keyboard->Release();
	Mouse->Release();

	ddsTex1->Release();
	ddsTex2->Release();
	ddsTex3->Release();
	normalMap->Release();
	AdsResourceView->Release();
	NpcRV->Release();
	gVertexBuffer2->Release();


	m_depthDisabledStencilState->Release();
	m_depthStencilState->Release();
	CWCullmode->Release();
	counterCWCullmode->Release();
	d2dIndexBuffer->Release();

	//Reader release
	fileMapReader->releaseStuff();

}

//Input handler for Mouse and Keyboard
bool Core::InitDirectInput(HINSTANCE hInstance)
{
	HRESULT hr;
	hr = DirectInput8Create(hInstance,
		DIRECTINPUT_VERSION,
		IID_IDirectInput8,
		(void**)&DirectInput,
		NULL);

	hr = DirectInput->CreateDevice(GUID_SysKeyboard,
		&Keyboard,
		NULL);

	hr = DirectInput->CreateDevice(GUID_SysMouse,
		&Mouse,
		NULL);

	hr = Keyboard->SetDataFormat(&c_dfDIKeyboard);
	hr = Keyboard->SetCooperativeLevel(hWindow, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);

	hr = Mouse->SetDataFormat(&c_dfDIMouse);
	hr = Mouse->SetCooperativeLevel(hWindow, DISCL_EXCLUSIVE | DISCL_NOWINKEY | DISCL_FOREGROUND);

	return true;
}

//INPUT HANDLER
void Core::InputHandler()
{
	//Keyboard and mouse handlers
	DIMOUSESTATE mouseCurrState;
	BYTE keyboardState[256];

	//Aquaire devices
	Keyboard->Acquire();
	Mouse->Acquire();

	//Get States
	Mouse->GetDeviceState(sizeof(DIMOUSESTATE), &mouseCurrState);
	Keyboard->GetDeviceState(sizeof(keyboardState), (LPVOID)&keyboardState);


	//Boost to move faster
	if (keyboardState[DIK_LSHIFT] & 0x80)
	{
		boost += 0.0002f;
	}
	else
		boost = 0;
	speedMultiplier = 50;
	//Keyboard and mouse interaction


	if (keyboardState[DIK_SPACE] & 0x80){
		speedMultiplier = 200;
	}


	if ((keyboardState[DIK_B] & 0x80) && Bcullingcheck == FALSE)
	{
		Bculling = TRUE;
		Bcullingcheck = TRUE;
	}
	if ((keyboardState[DIK_V] & 0x80) && Bcullingcheck == TRUE)
	{
		Bculling = FALSE;
		Bcullingcheck = FALSE;
	}

	if (keyboardState[DIK_A] & 0x80)
	{
		moveLeftRight -= speed;
	}
	if (keyboardState[DIK_D] & 0x80)
	{
		moveLeftRight += speed;
	}
	if (keyboardState[DIK_W] & 0x80)
	{
		moveBackForward += speed;

	}
	if (keyboardState[DIK_S] & 0x80)
	{
		moveBackForward -= speed;
	}
	if ((mouseCurrState.lX != mouseLastState.lX) || (mouseCurrState.lY != mouseLastState.lY))
	{
		camYaw += mouseCurrState.lX * speed / 4;;

		camPitch += mouseCurrState.lY * speed / 4;

		mouseLastState = mouseCurrState;
	}
	if (keyboardState[DIK_F1] & 0x80)
	{
		optionStruct.option1 = 1;
	};

	if (keyboardState[DIK_F2] & 0x80)
	{
		optionStruct.option1 = 0;
	};
	if (keyboardState[DIK_F3] & 0x80)
	{
		optionStruct.option2 = 1;
	};
	if (keyboardState[DIK_F4] & 0x80)
	{
		optionStruct.option2 = 0;
	}
	if (keyboardState[DIK_F5] & 0x80)
	{

		optionStruct.option5 = 1;
	}
	if (keyboardState[DIK_F6] & 0x80)
	{
		optionStruct.option6 = 1;
	}
	if (keyboardState[DIK_F6] & 0x80)
	{
		optionStruct.option7 = 1;
	}
	if (keyboardState[DIK_F7] & 0x80)
	{
		optionStruct.option7 = 0;
	}
	if (keyboardState[DIK_H] & 0x80)
	{
		highttest = 1;
	}
	if (keyboardState[DIK_H] & 0x80 && keyboardState[DIK_LALT] & 0x80)
	{
		highttest = 0;
	}
	if (keyboardState[DIK_F8] & 0x80)
	{
	//	camPosition = Vector4(0, 15, -15, 1);
		optionStruct.option8 = 1;
	}
	if (keyboardState[DIK_R] & 0x80 && keyboardState[DIK_LCONTROL] & 0x80)
	{
		renderBool = 1;
	}
	if (keyboardState[DIK_R] & 0x80 && keyboardState[DIK_LALT] & 0x80)
	{
		renderBool = 0;
	}
	if (keyboardState[DIK_H] & 0x80 && keyboardState[DIK_LCONTROL] & 0x80)
	{
		renderBool = 2;
	}
	if (keyboardState[DIK_G] & 0x80 && keyboardState[DIK_LCONTROL] & 0x80)
	{
		renderBool = 4;
	}
		
		if (keyboardState[DIK_I] &  0x80)
		{
			timeIterator += 0.002f;
			
		}
		if (keyboardState[DIK_I] & 0x80 && keyboardState[DIK_LCONTROL] & 0x80)
		{
			timeIterator += 0.002f;
		}
		if (keyboardState[DIK_I] & 0x80 && keyboardState[DIK_LSHIFT] & 0x80)
		{
		
		}



}

//ENABLE DEPTH
void Core::TurnZBufferOn()
{

	gDeviceContext->OMSetDepthStencilState(m_depthStencilState, 1);
	return;
}

//DISABLE DEPTH
void Core::TurnZBufferOff()
{

	gDeviceContext->OMSetDepthStencilState(m_depthDisabledStencilState, 1);
	return;
}

void Core::AlphaBlendOn()
{
	float blendFactors[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	gDeviceContext->OMSetBlendState(gBlendStateDefault, blendFactors, 0xffffffff);
}

void Core::AlphaBlendOff(){

	float blendFactors[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
	gDeviceContext->OMSetBlendState(gBlendStateTransparency, blendFactors, 0xffffffff);

}

//BLENDSTATES FOR TRANSPARENCY
void Core::BlendStates(){
	HRESULT hr;

	D3D11_BLEND_DESC bDesc;
	ZeroMemory(&bDesc, sizeof(bDesc));

	D3D11_RENDER_TARGET_BLEND_DESC rtbDesc;
	ZeroMemory(&rtbDesc, sizeof(rtbDesc));

	rtbDesc.BlendEnable = true;
	rtbDesc.SrcBlend = D3D11_BLEND_SRC_COLOR;
	rtbDesc.DestBlend = D3D11_BLEND_BLEND_FACTOR;
	rtbDesc.BlendOp = D3D11_BLEND_OP_ADD;
	rtbDesc.SrcBlendAlpha = D3D11_BLEND_ONE;
	rtbDesc.DestBlendAlpha = D3D11_BLEND_ZERO;
	rtbDesc.BlendOpAlpha = D3D11_BLEND_OP_ADD;
	rtbDesc.RenderTargetWriteMask = D3D10_COLOR_WRITE_ENABLE_ALL;

	bDesc.AlphaToCoverageEnable = false;
	bDesc.RenderTarget[0] = rtbDesc;

	gDevice->CreateBlendState(&bDesc, &transparency);

	//cull counter/clockwise
	D3D11_RASTERIZER_DESC cmdesc;
	ZeroMemory(&cmdesc, sizeof(D3D11_RASTERIZER_DESC));

	cmdesc.FillMode = D3D11_FILL_SOLID;
	cmdesc.CullMode = D3D11_CULL_BACK;

	cmdesc.FrontCounterClockwise = true;
	hr = gDevice->CreateRasterizerState(&cmdesc, &counterCWCullmode);

	cmdesc.FrontCounterClockwise = false;
	hr = gDevice->CreateRasterizerState(&cmdesc, &CWCullmode);
}

void Core::makelights(){
	for (int i = 1; i < 11; i++){

		//Randomized values
		float r = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float r2 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		float r3 = static_cast <float> (rand()) / static_cast <float> (RAND_MAX);
		int qt = rand() % 50 + 1;
		int qt2 = rand() % 50 + 1;
		qt = qt - 25;
		qt2 = qt2 - 25;

		//Set light values
		PrimaryLights.pLights[i - 1].Ambient = XMFLOAT4(0.2f, 0.2f, 0.2f, 1);
		PrimaryLights.pLights[i - 1].Specular = XMFLOAT4(0.3f, 0.3f, 0.3f, 0.0f);
		PrimaryLights.pLights[i - 1].Pos = XMFLOAT3(float(qt), 2.0f, float(qt2));
		PrimaryLights.pLights[i - 1].Diffuse = XMFLOAT4(r, r2, r3, 1.0f);
		PrimaryLights.pLights[i - 1].Range = 1.0f;
		PrimaryLights.pLights[i].Attenuation = XMFLOAT3(0.01f, 0.01f, 0.01f);
	}



	/*D3D11_BLEND_DESC alphaToCoverageDesc = { 0 };
	alphaToCoverageDesc.AlphaToCoverageEnable = true;
	alphaToCoverageDesc.IndependentBlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].BlendEnable = false;
	alphaToCoverageDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	gDevice->CreateBlendState(&alphaToCoverageDesc, &AlphaToCoverageBS);
*/
};

void Core::SetFonts(){

	AlphaBlendOn();
	TurnZBufferOff();
	spriteBatch->Begin();

	//std::string tValue(fileMapReader->meshStruct.meshName.nameString);// , fileMapReader->meshStruct.meshName.lenghtOfString);
	std::string MeshNumer = to_string(fileMapReader->meshes.size());
	std::wstringstream cls;
	cls << TheFps << "\nMeshes in scene: " << MeshNumer.c_str();// << "\n" << tValue.c_str();
	std::wstring total = cls.str();
	const wchar_t* spriteFontText;
	spriteFontText = total.c_str();
	//if (fileMapReader->meshes.size() != 0)
	// spriteFontText = total.c_str() ;
	//else {
	//	spriteFontText = L"No meshes found in scene";
	//}
	
	spritefont->DrawString(spriteBatch.get(), spriteFontText, DirectX::SimpleMath::Vector2(0, 10));
	spriteBatch->End();

	AlphaBlendOff();
	TurnZBufferOn();

};


