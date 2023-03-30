// header files
#include<windows.h>
#include<stdlib.h>	// for exit()
#include<stdio.h>	// for file I/O functions

#include<d3d11.h>   // main Direct3D 11 header
#include<dxgi.h>    // DirectX graphics infrastructure
#include<d3dcompiler.h>  // Direct3D HLSL compiler

#include"D3D.h"

#pragma warning(disable: 4838)  // suppressed warning: conversion from 'unsigned int' to 'INT' requires a narrowing conversion
#include"XNAMath/xnamath.h"  // XBox's Math Library

#include"Sphere.h"  // Sphere

// linking D3D libraries
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// linking Sphere library (if not done already, copy .\lib\bin\x64\Sphere.dll to C:\Windows\System32\)
#pragma comment(lib, "./lib/bin/x64/Sphere.lib")

// macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

#define ROT_NONE   0
#define ROT_AXIS_X 1
#define ROT_AXIS_Y 2
#define ROT_AXIS_Z 3

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global variable declarations
FILE *gpLog = NULL;
HWND ghwnd = NULL;
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE;

// D3D11 global variables
IDXGISwapChain *gpIDXGISwapChain = NULL;
ID3D11Device *gpID3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;
ID3D11DepthStencilView *gpID3D11DepthStencilView = NULL;
FLOAT clearColor[4];

// shaders
ID3D11VertexShader *gpID3D11VertexShader = NULL;
ID3D11PixelShader *gpID3D11PixelShader = NULL;

// input layout: specifies 'in', 'out', 'layout', etc. (of OpenGL) in shaders
ID3D11InputLayout *gpID3D11InputLayout = NULL;

// buffers
ID3D11Buffer *gpID3D11Buffer_PositionBuffer = NULL;
ID3D11Buffer *gpID3D11Buffer_NormalBuffer = NULL;
ID3D11Buffer *gpID3D11Buffer_IndexBuffer = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstantBuffer = NULL;  // uniform buffer in OpenGL

// to disable backface culling
ID3D11RasterizerState *gpID3D11RasterizerState = NULL;

// CPU-side uniform storage
struct CBuffer {
	XMMATRIX WorldMatrix;
	XMMATRIX ViewMatrix;
	XMMATRIX ProjectionMatrix;

	XMVECTOR LightLa;
	XMVECTOR LightLd;
	XMVECTOR LightLs;
	XMVECTOR LightPosition;

	XMVECTOR MaterialKa;
	XMVECTOR MaterialKd;
	XMVECTOR MaterialKs;
	FLOAT MaterialShininess;
	
	UINT LightIsEnabled;
};

// lighting
struct Light {
	XMVECTOR La;
	XMVECTOR Ld;
	XMVECTOR Ls;
	XMVECTOR Position;
	BOOL bIsEnabled;
} gLight;

// projection matrix
XMMATRIX PerspectiveProjectionMatrix;

// sphere data
UINT gNumVertices, gNumIndices;

// for rendering
FLOAT theta = 0.0f;
UINT windowWidth, windowHeight;
UINT rotationAxis = 0;

// entry-point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// function prototypes
	HRESULT initialize(void);
	void display(void);
	void update(void);
	void uninitialize(void);

	// variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyWindow");
	BOOL bDone = FALSE;
	HRESULT hr = S_OK;
	int cxScreen, cyScreen;

	// code
	fopen_s(&gpLog, "Log.txt", "w");
	fprintf(gpLog, "WinMain: created log file\n", hr);
	fclose(gpLog);

	// initialization of the wndclass structure
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(hInstance, MAKEINTRESOURCE(MYICON));

	// registering the wndclass
	RegisterClassEx(&wndclass);

	// getting the screen size
	cxScreen = GetSystemMetrics(SM_CXSCREEN);
	cyScreen = GetSystemMetrics(SM_CYSCREEN);

	// create the window
	hwnd = CreateWindowEx(WS_EX_APPWINDOW,
		szAppName,
		TEXT("D3D11: Kaivalya Vishwakumar Deshpande"),
		WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN | WS_CLIPSIBLINGS | WS_VISIBLE,
		(cxScreen - WIN_WIDTH) / 2,
		(cyScreen - WIN_HEIGHT) / 2,
		WIN_WIDTH,
		WIN_HEIGHT,
		NULL,
		NULL,
		hInstance,
		NULL);
	ghwnd = hwnd;

	// initialize
	hr = initialize();
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize() failed with error code 0x%lx\n", hr);
		fclose(gpLog);

		DestroyWindow(hwnd);
		hwnd = NULL;
	}
	else
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "WinMain: initialize succeeded\n", hr);
		fclose(gpLog);

		ShowWindow(hwnd, iCmdShow);

		SetForegroundWindow(hwnd);
		SetFocus(hwnd);
	}

	// game loop
	while (bDone != TRUE)
	{
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
				bDone = TRUE;
			else
			{
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		else
		{
			if (gbActiveWindow)
			{
				update();
				display();
			}
		}
	}

	// uninitialize
	uninitialize();

	return (int)msg.wParam;
}

// callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// function prototypes
	void ToggleFullScreen(void);
	HRESULT resize(int, int);

	// variable declarations
	HRESULT hr = S_OK;

	// code
	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActiveWindow = TRUE;

		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "WndProc: window in focus\n");
		fclose(gpLog);
		break;
	case WM_KILLFOCUS:
		gbActiveWindow = FALSE;

		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "WndProc: window out of focus\n");
		fclose(gpLog);
		break;
	case WM_ERASEBKGND:
		// because this is still retained mode graphics, there is WM_PAINT to paint
		// in the case of immediate mode graphics, we shall return 0 here
		break;
	case WM_KEYDOWN:
		switch (wParam)
		{
		case 27:
			fopen_s(&gpLog, "Log.txt", "a+");
			fprintf(gpLog, "WndProc: destroying after receiving esc\n");
			fclose(gpLog);

			DestroyWindow(hwnd);
			break;
		default:
			break;
		}
		break;
	case WM_CHAR:
		switch (wParam)
		{
		case 'F':
		case 'f':
			ToggleFullScreen();
			break;
		case 'L':
		case 'l':
			gLight.bIsEnabled = (gLight.bIsEnabled ? FALSE : TRUE);
			break;
		case 'X':
		case 'x':
			rotationAxis = ROT_AXIS_X;
			break;
		case 'Y':
		case 'y':
			rotationAxis = ROT_AXIS_Y;
			break;
		case 'Z':
		case 'z':
			rotationAxis = ROT_AXIS_Z;
			break;
		default:
			rotationAxis = ROT_NONE;
			break;
		}
		break;
	case WM_SIZE:
		if (gpID3D11DeviceContext)
		{
			hr = resize(LOWORD(lParam), HIWORD(lParam));
			if (FAILED(hr))
			{
				fopen_s(&gpLog, "Log.txt", "a+");
				fprintf(gpLog, "WndProc: resize failed with error code 0x%lx\n", hr);
				fclose(gpLog);

				DestroyWindow(hwnd);
				hwnd = NULL;
			}
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hwnd);
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}

void ToggleFullScreen(void)
{
	// variable declarations
	static DWORD dwStyle;
	static WINDOWPLACEMENT wp;
	MONITORINFO mi;

	// code
	wp.length = sizeof(WINDOWPLACEMENT);

	if (gbFullScreen == FALSE)
	{
		dwStyle = GetWindowLong(ghwnd, GWL_STYLE);

		if (dwStyle & WS_OVERLAPPEDWINDOW)
		{
			mi.cbSize = sizeof(MONITORINFO);

			if (GetWindowPlacement(ghwnd, &wp) && GetMonitorInfo(MonitorFromWindow(ghwnd, MONITORINFOF_PRIMARY), &mi))
			{
				SetWindowLong(ghwnd, GWL_STYLE, dwStyle & ~WS_OVERLAPPEDWINDOW);
				SetWindowPos(ghwnd, HWND_TOP, mi.rcMonitor.left, mi.rcMonitor.top,
					mi.rcMonitor.right - mi.rcMonitor.left, mi.rcMonitor.bottom - mi.rcMonitor.top,
					SWP_NOZORDER | SWP_FRAMECHANGED);
			}

			ShowCursor(FALSE);
			gbFullScreen = TRUE;

			fopen_s(&gpLog, "Log.txt", "a+");
			fprintf(gpLog, "ToggleFullScreen: fullscreen mode on\n");
			fclose(gpLog);
		}
	}
	else
	{
		SetWindowLong(ghwnd, GWL_STYLE, dwStyle | WS_OVERLAPPEDWINDOW);
		SetWindowPlacement(ghwnd, &wp);
		SetWindowPos(ghwnd, HWND_TOP, 0, 0, 0, 0,
			SWP_NOMOVE | SWP_NOSIZE | SWP_NOOWNERZORDER | SWP_NOZORDER | SWP_FRAMECHANGED);

		ShowCursor(TRUE);
		gbFullScreen = FALSE;

		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "ToggleFullScreen: fullscreen mode off\n");
		fclose(gpLog);
	}
}

HRESULT initialize(void)
{
	// function prototypes
	HRESULT PrintD3DInfo(void);
	HRESULT resize(int, int);

	// variable declarations
	HRESULT hr = S_OK;

	D3D_DRIVER_TYPE d3dDriverType;
	D3D_DRIVER_TYPE d3dDriverTypes[] = {
		D3D_DRIVER_TYPE_HARDWARE,  // hardware vendor-supplied driver (most performant)
		D3D_DRIVER_TYPE_WARP,  // Microsoft's generic driver (Windows Advanced Rasterizer Platform) (decent performance)
		D3D_DRIVER_TYPE_REFERENCE  // driver for debugging purposes
	};
	
	D3D_FEATURE_LEVEL d3dFeatureLevel_required = D3D_FEATURE_LEVEL_11_0;  // require D3D 11.0
	D3D_FEATURE_LEVEL d3dFeatureLevel_acquired = D3D_FEATURE_LEVEL_10_0;  // as a fallback

	UINT numDrivers = 0U;
	UINT deviceCreationFlags = 0U;
	UINT numFeatureLevels = 1U;

	RECT rc;

	// code
	// log D3D info
	hr = PrintD3DInfo();
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: PrintD3DInfo failed with error code 0x%lx\n", hr);
		fclose(gpLog);
		return hr;
	}

	// initialize swap chain descriptor
	DXGI_SWAP_CHAIN_DESC dxgiSwapChainDescriptor;
	ZeroMemory((void *)&dxgiSwapChainDescriptor, sizeof(dxgiSwapChainDescriptor));
	dxgiSwapChainDescriptor.BufferCount = 1U;  // D3D provides another dedicated buffer itself, making total = 2
	dxgiSwapChainDescriptor.BufferDesc.Width = WIN_WIDTH;
	dxgiSwapChainDescriptor.BufferDesc.Height = WIN_HEIGHT;
	dxgiSwapChainDescriptor.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;  // unsigned, normalized
	dxgiSwapChainDescriptor.BufferDesc.RefreshRate.Numerator = 60U;  // Refresh rate = 60/1 = 60 Hz
	dxgiSwapChainDescriptor.BufferDesc.RefreshRate.Denominator = 1U;
	dxgiSwapChainDescriptor.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;  // use these buffers as RTVs
	dxgiSwapChainDescriptor.OutputWindow = ghwnd;
	dxgiSwapChainDescriptor.SampleDesc.Count = 1U;  // no multi sampling
	dxgiSwapChainDescriptor.SampleDesc.Quality = 0U;  // default quality
	dxgiSwapChainDescriptor.Windowed = TRUE;  // show Windowed buffer first, not fullscreen

	// create device and swap chain for the required driver
	numDrivers = sizeof(d3dDriverTypes) / sizeof(d3dDriverTypes[0]);
	for (UINT i = 0U; i < numDrivers; i++)
	{
		d3dDriverType = d3dDriverTypes[i];

		hr = D3D11CreateDeviceAndSwapChain(
			NULL,  // on primary GPU device adapter
			d3dDriverType,
			NULL,  // no software rasterizer DLL
			deviceCreationFlags,
			&d3dFeatureLevel_required,
			numFeatureLevels,
			D3D11_SDK_VERSION,
			&dxgiSwapChainDescriptor,
			&gpIDXGISwapChain,
			&gpID3D11Device,
			&d3dFeatureLevel_acquired,
			&gpID3D11DeviceContext
		);
		if (SUCCEEDED(hr))
			break;
	}
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: D3D11CreateDeviceAndSwapChain failed with error code 0x%lx\n", hr);
		fclose(gpLog);
		return hr;
	}
	
	// log obtained driver type and D3D feature level
	fopen_s(&gpLog, "Log.txt", "a+");
	switch (d3dDriverType)
	{
	case D3D_DRIVER_TYPE_HARDWARE:
		fprintf(gpLog, "initialize: D3D11CreateDeviceAndSwapChain obtained Hardware Driver\n", hr);
		break;
	case D3D_DRIVER_TYPE_WARP:
		fprintf(gpLog, "initialize: D3D11CreateDeviceAndSwapChain obtained WARP Driver\n", hr);
		break;
	case D3D_DRIVER_TYPE_REFERENCE:
		fprintf(gpLog, "initialize: D3D11CreateDeviceAndSwapChain obtained Reference Driver\n", hr);
		break;
	default:
		fprintf(gpLog, "initialize: D3D11CreateDeviceAndSwapChain obtained an unknown Driver\n", hr);
		break;
	}

	switch (d3dFeatureLevel_acquired)
	{
	case D3D_FEATURE_LEVEL_11_0:
		fprintf(gpLog, "initialize: D3D11CreateDeviceAndSwapChain obtained Feature Level 11.0\n", hr);
		break;
	case D3D_FEATURE_LEVEL_10_1:
		fprintf(gpLog, "initialize: D3D11CreateDeviceAndSwapChain obtained Feature Level 10.1\n", hr);
		break;
	case D3D_FEATURE_LEVEL_10_0:
		fprintf(gpLog, "initialize: D3D11CreateDeviceAndSwapChain obtained Feature Level 10.0\n", hr);
		break;
	default:
		fprintf(gpLog, "initialize: D3D11CreateDeviceAndSwapChain obtained an unknown Feature Level\n", hr);
		break;
	}
	fclose(gpLog);

	// vertex shader
	const char *vertexShaderSourceCode =
		"cbuffer constantBuffer\n" \
		"{\n" \
		"	float4x4 worldMatrix;\n" \
		"	float4x4 viewMatrix;\n" \
		"	float4x4 projectionMatrix;\n" \
		"	\n" \
		"	float4 lightLa;\n" \
		"	float4 lightLd;\n" \
		"	float4 lightLs;\n" \
		"	float4 lightPosition;\n" \
		"	\n" \
		"	float4 materialKa;\n" \
		"	float4 materialKd;\n" \
		"	float4 materialKs;\n" \
		"	float materialShininess;\n" \
		"	\n" \
		"	uint lightIsEnabled;\n" \
		"}\n" \
		"\n" \
		"struct Vertex\n" \
		"{\n" \
		"	float4 position: SV_POSITION;\n" \
		"	float3 lightDirection: VSOUT_LIGHT_DIRECTION;\n" \
		"	float3 normal: VSOUT_NORMAL;\n" \
		"	float3 viewerVector: VSOUT_VIEWER_VECTOR;\n" \
		"};\n" \
		"\n" \
		"Vertex main(float4 position: POSITION, float4 normal: NORMAL)\n" \
		"{\n" \
		"	Vertex output;\n" \
		"	if(lightIsEnabled != 0) {\n" \
		"		float4 eyeCoordinate = mul(viewMatrix, mul(worldMatrix, position));\n" \
		"		float3x3 normalMatrix = (float3x3)worldMatrix;\n" \
		"		output.lightDirection = lightPosition.xyz - eyeCoordinate.xyz;\n" \
		"		output.normal = mul(normalMatrix, normal.xyz);\n" \
		"		output.viewerVector = -eyeCoordinate.xyz;\n" \
		"	}\n" \
		"	output.position = mul(projectionMatrix, mul(viewMatrix, mul(worldMatrix, position)));\n" \
		"	return output;\n" \
		"}\n";

	// compile vertex shader
	ID3DBlob *pID3DBlob_VertexShaderCode = NULL;
	ID3DBlob *pID3DBlob_Error = NULL;
	hr = D3DCompile(
		vertexShaderSourceCode,
		lstrlenA(vertexShaderSourceCode) + 1,
		"VS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"vs_5_0",  // vertex shader feature level 5.0
		0,  // compile constants, like fast-relaxed-math in OpenCL
		0,	// effect constants, when using fxc.exe
		&pID3DBlob_VertexShaderCode,  // compiled code blob
		&pID3DBlob_Error  // error blob
	);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		if (pID3DBlob_Error)
		{
			fprintf(gpLog, "initialize: D3DCompile failed: vertex shader compilation errors\n");
			fprintf(gpLog, "%s\n", (char *)pID3DBlob_Error->GetBufferPointer());

			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
		}
		else
		{
			fprintf(gpLog, "initialize: D3DCompile failed: (vertex shader) there is nothing to print\n");
		}
		fclose(gpLog);

		// !!! no return until input layout is set !!!
	}
	else
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: D3DCompile succeeded for vertex shader\n");
		fclose(gpLog);
	}

	// create the vertex shader
	hr = gpID3D11Device->CreateVertexShader(
		pID3DBlob_VertexShaderCode->GetBufferPointer(),
		pID3DBlob_VertexShaderCode->GetBufferSize(),
		NULL,
		&gpID3D11VertexShader
	);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: CreateVertexShader failed with error code 0x%lx\n", hr);
		fclose(gpLog);
	}
	else
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: CreateVertexShader succeeded\n");
		fclose(gpLog);
	}

	// set this vertex shader in the vertex shader stage of pipeline
	gpID3D11DeviceContext->VSSetShader(gpID3D11VertexShader, NULL, 0);

	// pixel shader
	const char *pixelShaderSourceCode =
		"cbuffer constantBuffer\n" \
		"{\n" \
		"	float4x4 worldMatrix;\n" \
		"	float4x4 viewMatrix;\n" \
		"	float4x4 projectionMatrix;\n" \
		"	\n" \
		"	float4 lightLa;\n" \
		"	float4 lightLd;\n" \
		"	float4 lightLs;\n" \
		"	float4 lightPosition;\n" \
		"	\n" \
		"	float4 materialKa;\n" \
		"	float4 materialKd;\n" \
		"	float4 materialKs;\n" \
		"	float materialShininess;\n" \
		"	\n" \
		"	uint lightIsEnabled;\n" \
		"}\n" \
		"\n" \
		"struct Vertex\n" \
		"{\n" \
		"	float4 position: SV_POSITION;\n" \
		"	float3 lightDirection: VSOUT_LIGHT_DIRECTION;\n" \
		"	float3 normal: VSOUT_NORMAL;\n" \
		"	float3 viewerVector: VSOUT_VIEWER_VECTOR;\n" \
		"};\n" \
		"\n" \
		"float4 main(Vertex input): SV_TARGET\n" \
		"{\n" \
		"	if (lightIsEnabled != 0) {\n" \
		"		float3 lightDirection = normalize(input.lightDirection);\n" \
		"		float3 transformedNormal = normalize(input.normal);\n" \
		"		float3 viewerVector = normalize(input.viewerVector);\n" \
		"		float3 reflectedDirection = reflect(-lightDirection, transformedNormal);\n" \
		"		\n" \
		"		float4 ambient = lightLa * materialKa;\n" \
		"		float4 diffuse = lightLd * materialKd * max(dot(lightDirection, transformedNormal), 0.0f);\n" \
		"		float4 specular = lightLs * materialKs * pow(max(dot(viewerVector, reflectedDirection), 0.0f), materialShininess);\n" \
		"		\n" \
		"		return (ambient + diffuse + specular);\n" \
		"	}\n" \
		"	return float4(1.0f, 1.0f, 1.0f, 1.0f);\n" \
		"}\n";

	// compile pixel shader
	ID3DBlob *pID3DBlob_PixelShaderCode = NULL;
	pID3DBlob_Error = NULL;
	hr = D3DCompile(
		pixelShaderSourceCode,
		lstrlenA(pixelShaderSourceCode) + 1,
		"PS",
		NULL,
		D3D_COMPILE_STANDARD_FILE_INCLUDE,
		"main",
		"ps_5_0",  // pixel shader feature level 5.0
		0,
		0,
		&pID3DBlob_PixelShaderCode,
		&pID3DBlob_Error
	);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		if (pID3DBlob_Error)
		{
			fprintf(gpLog, "initialize: D3DCompile failed: pixel shader compilation errors\n");
			fprintf(gpLog, "%s\n", (char *)pID3DBlob_Error->GetBufferPointer());

			pID3DBlob_Error->Release();
			pID3DBlob_Error = NULL;
		}
		else
		{
			fprintf(gpLog, "initialize: D3DCompile failed: (pixel shader) there is nothing to print\n");
		}
		fclose(gpLog);
	}
	else
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: D3DCompile succeeded for pixel shader\n");
		fclose(gpLog);
	}

	// create the pixel shader
	hr = gpID3D11Device->CreatePixelShader(
		pID3DBlob_PixelShaderCode->GetBufferPointer(),
		pID3DBlob_PixelShaderCode->GetBufferSize(),
		NULL,
		&gpID3D11PixelShader
	);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: CreatePixelShader failed with error code 0x%lx\n", hr);
		fclose(gpLog);
	}
	else
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: CreatePixelShader succeeded\n");
		fclose(gpLog);
	}

	// set this pixel shader in the pixel shader stage of pipeline
	gpID3D11DeviceContext->PSSetShader(gpID3D11PixelShader, NULL, 0);

	// configure input layout
	D3D11_INPUT_ELEMENT_DESC d3d11InputElementDescriptor[2];
	ZeroMemory((void *)d3d11InputElementDescriptor, sizeof(d3d11InputElementDescriptor));
	
	// Vertex.position
	d3d11InputElementDescriptor[0].SemanticName = "POSITION";
	d3d11InputElementDescriptor[0].Format = DXGI_FORMAT_R32G32B32_FLOAT;  // pass as a float3
	d3d11InputElementDescriptor[0].InputSlot = 0;
	d3d11InputElementDescriptor[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// Vertex.normal
	d3d11InputElementDescriptor[1].SemanticName = "NORMAL";
	d3d11InputElementDescriptor[1].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	d3d11InputElementDescriptor[1].InputSlot = 1;
	d3d11InputElementDescriptor[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// create input layout
	hr = gpID3D11Device->CreateInputLayout(
		d3d11InputElementDescriptor,
		_ARRAYSIZE(d3d11InputElementDescriptor),
		pID3DBlob_VertexShaderCode->GetBufferPointer(),
		pID3DBlob_VertexShaderCode->GetBufferSize(),
		&gpID3D11InputLayout
	);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: CreateInputLayout failed with error code 0x%lx\n", hr);
		fclose(gpLog);
	}
	else
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: CreateInputLayout succeeded\n");
		fclose(gpLog);
	}

	// set this input layout in the input assembly stage of pipeline
	gpID3D11DeviceContext->IASetInputLayout(gpID3D11InputLayout);

	// release the VS and PS blobs
	if (pID3DBlob_PixelShaderCode)
	{
		pID3DBlob_PixelShaderCode->Release();
		pID3DBlob_PixelShaderCode = NULL;
	}
	if (pID3DBlob_VertexShaderCode)
	{
		pID3DBlob_VertexShaderCode->Release();
		pID3DBlob_VertexShaderCode = NULL;
	}

	// geometry
	float vertexPositions[1146];
	float vertexNormals[1146];
	float vertexTexCoords[764];
	unsigned short indices[2280];
	getSphereVertexData(
		vertexPositions,
		vertexNormals,
		vertexTexCoords,
		indices
	);
	gNumVertices = getNumberOfSphereVertices();
	gNumIndices = getNumberOfSphereElements();

	// positions buffer
	// configure
	D3D11_BUFFER_DESC d3d11BufferDescriptor;
	ZeroMemory((void *)&d3d11BufferDescriptor, sizeof(d3d11BufferDescriptor));
	d3d11BufferDescriptor.Usage = D3D11_USAGE_DEFAULT;  // GPU Readable/Writable
	d3d11BufferDescriptor.ByteWidth = sizeof(float) * 3 * gNumVertices;
	d3d11BufferDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	// populate
	D3D11_SUBRESOURCE_DATA d3d11SubresourceData;
	ZeroMemory((void *)&d3d11SubresourceData, sizeof(d3d11SubresourceData));
	d3d11SubresourceData.pSysMem = vertexPositions;

	// create
	hr = gpID3D11Device->CreateBuffer(&d3d11BufferDescriptor, &d3d11SubresourceData, &gpID3D11Buffer_PositionBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: CreateBuffer for Position Buffer failed with error code 0x%lx\n", hr);
		fclose(gpLog);
	}
	else
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: CreateBuffer for Position Buffer succeeded\n");
		fclose(gpLog);
	}

	// normals buffer
	// configure
	ZeroMemory((void *)&d3d11BufferDescriptor, sizeof(d3d11BufferDescriptor));
	d3d11BufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
	d3d11BufferDescriptor.ByteWidth = sizeof(float) * 3 * gNumVertices;
	d3d11BufferDescriptor.BindFlags = D3D11_BIND_VERTEX_BUFFER;

	// populate
	ZeroMemory((void *)&d3d11SubresourceData, sizeof(d3d11SubresourceData));
	d3d11SubresourceData.pSysMem = vertexNormals;

	// create
	hr = gpID3D11Device->CreateBuffer(&d3d11BufferDescriptor, &d3d11SubresourceData, &gpID3D11Buffer_NormalBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: CreateBuffer for Normal Buffer failed with error code 0x%lx\n", hr);
		fclose(gpLog);
	}
	else
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: CreateBuffer for Normal Buffer succeeded\n");
		fclose(gpLog);
	}

	// index buffer
	// configure
	ZeroMemory((void *)&d3d11BufferDescriptor, sizeof(d3d11BufferDescriptor));
	d3d11BufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
	d3d11BufferDescriptor.ByteWidth = sizeof(unsigned short) * gNumIndices;
	d3d11BufferDescriptor.BindFlags = D3D11_BIND_INDEX_BUFFER;

	// populate
	ZeroMemory((void *)&d3d11SubresourceData, sizeof(d3d11SubresourceData));
	d3d11SubresourceData.pSysMem = indices;

	// create
	hr = gpID3D11Device->CreateBuffer(&d3d11BufferDescriptor, &d3d11SubresourceData, &gpID3D11Buffer_IndexBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: CreateBuffer for Index Buffer failed with error code 0x%lx\n", hr);
		fclose(gpLog);
	}
	else
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: CreateBuffer for Index Buffer succeeded\n");
		fclose(gpLog);
	}

	// constant buffer
	// configure
	ZeroMemory((void *)&d3d11BufferDescriptor, sizeof(d3d11BufferDescriptor));
	d3d11BufferDescriptor.Usage = D3D11_USAGE_DEFAULT;
	d3d11BufferDescriptor.ByteWidth = sizeof(CBuffer);
	d3d11BufferDescriptor.BindFlags = D3D11_BIND_CONSTANT_BUFFER;

	// create
	hr = gpID3D11Device->CreateBuffer(&d3d11BufferDescriptor, NULL, &gpID3D11Buffer_ConstantBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: CreateBuffer for Constant Buffer failed with error code 0x%lx\n", hr);
		fclose(gpLog);
	}
	else
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: CreateBuffer for Constant Buffer succeeded\n");
		fclose(gpLog);
	}

	// set into vertex and pixel shader stage of pipeline
	gpID3D11DeviceContext->VSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);
	gpID3D11DeviceContext->PSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);

	// cull-disabling rasterizer state
	D3D11_RASTERIZER_DESC d3d11RasterizerDescriptor;
	ZeroMemory((void *)&d3d11RasterizerDescriptor, sizeof(d3d11RasterizerDescriptor));
	d3d11RasterizerDescriptor.CullMode = D3D11_CULL_NONE;
	d3d11RasterizerDescriptor.FillMode = D3D11_FILL_SOLID;
	d3d11RasterizerDescriptor.FrontCounterClockwise = FALSE;
	d3d11RasterizerDescriptor.MultisampleEnable = FALSE;
	d3d11RasterizerDescriptor.ScissorEnable = FALSE;
	d3d11RasterizerDescriptor.DepthClipEnable = TRUE;
	d3d11RasterizerDescriptor.AntialiasedLineEnable = FALSE;
	d3d11RasterizerDescriptor.DepthBias = 0.0f;
	d3d11RasterizerDescriptor.DepthBiasClamp = 0.0f;
	d3d11RasterizerDescriptor.SlopeScaledDepthBias = 0.0f;

	hr = gpID3D11Device->CreateRasterizerState(&d3d11RasterizerDescriptor, &gpID3D11RasterizerState);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: CreateRasterizerState failed with error code 0x%lx\n", hr);
		fclose(gpLog);
	}
	else
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: CreateRasterizerState succeeded\n");
		fclose(gpLog);
	}

	gpID3D11DeviceContext->RSSetState(gpID3D11RasterizerState);

	// initialize the clear color
	clearColor[0] = 0.25f;
	clearColor[1] = 0.25f;
	clearColor[2] = 0.25f;
	clearColor[3] = 1.0f;

	PerspectiveProjectionMatrix = XMMatrixIdentity();

	// initialize the light
	gLight.La = XMVectorSet(0.1f, 0.1f, 0.1f, 1.0f);
	gLight.Ld = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	gLight.Ls = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	gLight.Position = XMVectorSet(0.0f, 0.0f, -100.0f, 1.0f);
	gLight.bIsEnabled = FALSE;

	// warm-up resize
	GetClientRect(ghwnd, &rc);
	hr = resize(rc.right - rc.left, rc.bottom - rc.top);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "initialize: resize failed with error code 0x%lx\n", hr);
		fclose(gpLog);
		return hr;
	}
	
	fopen_s(&gpLog, "Log.txt", "a+");
	fprintf(gpLog, "initialize: resize succeeded\n", hr);
	fclose(gpLog);
	return hr;
}

HRESULT PrintD3DInfo(void)
{
	// variable declarations
	HRESULT hr = S_OK;
	IDXGIFactory *pIDXGIFactory = NULL;
	IDXGIAdapter *pIDXGIAdapter = NULL;
	DXGI_ADAPTER_DESC dxgiAdapterDescriptor;
	char str[255];

	// code
	// create a DXGIFactory
	hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pIDXGIFactory);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "PrintD3DInfo: CreateDXGIFactory() failed with error code 0x%lx\n", hr);
		fclose(gpLog);
		return hr;
	}
	else
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "PrintD3DInfo: CreateDXGIFactory() succeeded\n", hr);
		fclose(gpLog);
	}

	// get IDXGIAdapter (ex. interface to the 0th graphics device connected to the system) from the factory
	if ((hr = pIDXGIFactory->EnumAdapters(0, &pIDXGIAdapter)) != DXGI_ERROR_NOT_FOUND)
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "PrintD3DInfo: IDXGIFactory::EnumAdapters() for adapter at ordinal 0 succeeded\n");
		fclose(gpLog);

		// get description of the found adapter
		ZeroMemory((void *)&dxgiAdapterDescriptor, sizeof(dxgiAdapterDescriptor));
		pIDXGIAdapter->GetDesc(&dxgiAdapterDescriptor);

		// log the obtained information
		WideCharToMultiByte(
			CP_ACP,
			0,
			dxgiAdapterDescriptor.Description,
			255,
			str,
			255,
			NULL,
			NULL
		);
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "\n--------------------- D3D11 Properties --------------------\n\n");
		fprintf(gpLog, "Graphics Device        : %s\n", str);
		fprintf(gpLog, "Dedicated Video Memory : %d GiB\n", (int)ceil(dxgiAdapterDescriptor.DedicatedVideoMemory / 1024.0 / 1024.0 / 1024.0));
		fprintf(gpLog, "-----------------------------------------------------------\n\n");
		fclose(gpLog);
	}
	else
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "PrintD3DInfo: pIDXGIFactory::EnumAdapters() failed with error code 0x%lx\n", hr);
		fclose(gpLog);
	}

	if (pIDXGIAdapter)
	{
		pIDXGIAdapter->Release();
		pIDXGIAdapter = NULL;
	}
	if (pIDXGIFactory)
	{
		pIDXGIFactory->Release();
		pIDXGIFactory = NULL;
	}

	return hr;
}

HRESULT resize(int width, int height)
{
	// variable declarations
	HRESULT hr = S_OK;

	// code
	if (height <= 0)
		height = 1;

	// release existing DSV
	if (gpID3D11DepthStencilView)
	{
		gpID3D11DepthStencilView->Release();
		gpID3D11DepthStencilView = NULL;
	}

	// release existing RTV
	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}

	// ask swap chain to resize buffers according to new width and height
	hr = gpIDXGISwapChain->ResizeBuffers(1U, width, height, DXGI_FORMAT_R8G8B8A8_UNORM, 0U);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "resize: IDXGISwapChain::ResizeBuffers failed with error code 0x%lx\n", hr);
		fclose(gpLog);
		return hr;
	}

	// put the resized buffer from swap chain into a dummy texture
	ID3D11Texture2D *pID3D11Texture2D_BackBuffer = NULL;  // front buffer is already given by D3D
	hr = gpIDXGISwapChain->GetBuffer(0U, __uuidof(ID3D11Texture2D), (LPVOID *)&pID3D11Texture2D_BackBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "resize: IDXGISwapChain::GetBuffer failed with error code 0x%lx\n", hr);
		fclose(gpLog);
		return hr;
	}

	// create a new RTV using the new buffer
	hr = gpID3D11Device->CreateRenderTargetView(pID3D11Texture2D_BackBuffer, NULL, &gpID3D11RenderTargetView);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "resize: ID3D11Device::CreateRenderTargetView failed with error code 0x%lx\n", hr);
		fclose(gpLog);
		pID3D11Texture2D_BackBuffer->Release();
		pID3D11Texture2D_BackBuffer = NULL;
		return hr;
	}

	// release the dummy texture
	pID3D11Texture2D_BackBuffer->Release();
	pID3D11Texture2D_BackBuffer = NULL;

	// initialize a Texture2D descriptor
	D3D11_TEXTURE2D_DESC d3d11Texture2DDescriptor;
	ZeroMemory((void *)&d3d11Texture2DDescriptor, sizeof(d3d11Texture2DDescriptor));
	d3d11Texture2DDescriptor.Width = (UINT)width;
	d3d11Texture2DDescriptor.Height = (UINT)height;
	d3d11Texture2DDescriptor.ArraySize = 1U;
	d3d11Texture2DDescriptor.MipLevels = 1U;
	d3d11Texture2DDescriptor.SampleDesc.Count = 1U;
	d3d11Texture2DDescriptor.SampleDesc.Quality = 0U;  // default quailty
	d3d11Texture2DDescriptor.Usage = D3D11_USAGE_DEFAULT;
	d3d11Texture2DDescriptor.Format = DXGI_FORMAT_D32_FLOAT;
	d3d11Texture2DDescriptor.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	d3d11Texture2DDescriptor.CPUAccessFlags = 0U;  // default CPU access right
	d3d11Texture2DDescriptor.MiscFlags = 0U;  // no misc. flags

	// allocate a local depth buffer
	ID3D11Texture2D *pID3D11Texture2D_DepthBuffer = NULL;
	hr = gpID3D11Device->CreateTexture2D(&d3d11Texture2DDescriptor, NULL, &pID3D11Texture2D_DepthBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "resize: ID3D11Device::CreateTexture2D failed with error code 0x%lx\n", hr);
		fclose(gpLog);
		return hr;
	}

	// initialize DSV descriptor
	D3D11_DEPTH_STENCIL_VIEW_DESC d3d11DepthStencilViewDescriptor;
	ZeroMemory((void *)&d3d11DepthStencilViewDescriptor, sizeof(d3d11DepthStencilViewDescriptor));
	d3d11DepthStencilViewDescriptor.Format = DXGI_FORMAT_D32_FLOAT;
	d3d11DepthStencilViewDescriptor.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2DMS;  // in case multi-sampling is enabled

	// create a new DSV using the local depth buffer and descriptor
	hr = gpID3D11Device->CreateDepthStencilView(pID3D11Texture2D_DepthBuffer, &d3d11DepthStencilViewDescriptor, &gpID3D11DepthStencilView);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "resize: ID3D11Device::CreateDepthStencilView failed with error code 0x%lx\n", hr);
		fclose(gpLog);
		pID3D11Texture2D_DepthBuffer->Release();
		pID3D11Texture2D_DepthBuffer = NULL;
		return hr;
	}

	// release the local depth buffer
	pID3D11Texture2D_DepthBuffer->Release();
	pID3D11Texture2D_DepthBuffer = NULL;

	// set the new RTV and DSV in pipeline
	gpID3D11DeviceContext->OMSetRenderTargets(1U, &gpID3D11RenderTargetView, gpID3D11DepthStencilView);

	// reset projection matrix
	FLOAT aspectRatio = (FLOAT)(width / 4) / (FLOAT)(height / 6);
	PerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.0f),
		aspectRatio,
		0.1f,
		100.0f
	);

	windowWidth = width;
	windowHeight = height;

	return hr;
}

void display(void)
{
	// function prototypes
	void Draw24Spheres(void);

	// code
	// clear RTV
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, clearColor);
	gpID3D11DeviceContext->ClearDepthStencilView(gpID3D11DepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, (UINT8)0);

	// render the spheres
	Draw24Spheres();

	// swap buffers by presenting them
	gpIDXGISwapChain->Present(
		0U,  // don't try to sync with monitor refresh rate as 60 Hz is preset
		0U   // swap among all buffers
	);
}

void update(void)
{
	// code
	theta += 0.02f;
	if (theta > 360.0f)
		theta -= 360.0f;
}

void uninitialize(void)
{
	// function prototypes
	void ToggleFullScreen(void);

	// code
	if (gbFullScreen)
	{
		// disciplined code: before being destroyed, toggle back to the restored state
		ToggleFullScreen();
	}
	if (gpID3D11RasterizerState)
	{
		gpID3D11RasterizerState->Release();
		gpID3D11RasterizerState = NULL;
	}
	if (gpID3D11Buffer_ConstantBuffer)
	{
		gpID3D11Buffer_ConstantBuffer->Release();
		gpID3D11Buffer_ConstantBuffer = NULL;
	}
	if (gpID3D11Buffer_IndexBuffer)
	{
		gpID3D11Buffer_IndexBuffer->Release();
		gpID3D11Buffer_IndexBuffer = NULL;
	}
	if (gpID3D11Buffer_NormalBuffer)
	{
		gpID3D11Buffer_NormalBuffer->Release();
		gpID3D11Buffer_NormalBuffer = NULL;
	}
	if (gpID3D11Buffer_PositionBuffer)
	{
		gpID3D11Buffer_PositionBuffer->Release();
		gpID3D11Buffer_PositionBuffer = NULL;
	}
	if (gpID3D11InputLayout)
	{
		gpID3D11InputLayout->Release();
		gpID3D11InputLayout = NULL;
	}
	if (gpID3D11PixelShader)
	{
		gpID3D11PixelShader->Release();
		gpID3D11PixelShader = NULL;
	}
	if (gpID3D11VertexShader)
	{
		gpID3D11VertexShader->Release();
		gpID3D11VertexShader = NULL;
	}
	if (gpID3D11DepthStencilView)
	{
		gpID3D11DepthStencilView->Release();
		gpID3D11DepthStencilView = NULL;
	}
	if (gpID3D11RenderTargetView)
	{
		gpID3D11RenderTargetView->Release();
		gpID3D11RenderTargetView = NULL;
	}
	if (gpIDXGISwapChain)
	{
		gpIDXGISwapChain->Release();
		gpIDXGISwapChain = NULL;
	}
	if (gpID3D11DeviceContext)
	{
		gpID3D11DeviceContext->Release();
		gpID3D11DeviceContext = NULL;
	}
	if (gpID3D11Device)
	{
		gpID3D11Device->Release();
		gpID3D11Device = NULL;
	}
	if (ghwnd)
	{
		DestroyWindow(ghwnd);	// if unitialize() was not called from WM_DESTROY
		ghwnd = NULL;	// disciplined code: once a handle is invalid, reset to NULL
	}
}

void Draw24Spheres(void)
{
	// variable declarations
	CBuffer constantBuffer;
	D3D11_VIEWPORT d3d11Viewport;

	// code
	// set position buffer in input assembly stage of pipeline
	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_PositionBuffer, &stride, &offset);

	// set normal buffer in input assembly stage of pipeline
	stride = sizeof(float) * 3;
	offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(1, 1, &gpID3D11Buffer_NormalBuffer, &stride, &offset);

	// set index buffer in input assembly stage of pipeline
	offset = 0;
	gpID3D11DeviceContext->IASetIndexBuffer(gpID3D11Buffer_IndexBuffer, DXGI_FORMAT_R16_UINT, offset);

	// set primitive topology in input assembly stage of pipeline
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// load constant buffer data that won't change with spheres
	ZeroMemory((void *)&constantBuffer, sizeof(constantBuffer));
	constantBuffer.WorldMatrix = XMMatrixTranslation(0.0f, 0.0f, 2.0f);
	constantBuffer.ViewMatrix = XMMatrixIdentity();
	constantBuffer.ProjectionMatrix = PerspectiveProjectionMatrix;

	// set viewport data that won't change with spheres
	d3d11Viewport.Width = (FLOAT)(windowWidth / 4);
	d3d11Viewport.Height = (FLOAT)(windowHeight / 6);
	d3d11Viewport.MinDepth = 0.0f;
	d3d11Viewport.MaxDepth = 1.0f;

	if (gLight.bIsEnabled)
	{
		// enable the lights
		constantBuffer.LightLa = gLight.La;
		constantBuffer.LightLd = gLight.Ld;
		constantBuffer.LightLs = gLight.Ls;
		constantBuffer.LightIsEnabled = 1U;

		// update light position
		switch (rotationAxis)
		{
		case ROT_NONE:
			gLight.Position = XMVectorSet(0.0f, 0.0f, -10.0f, 1.0f);
			break;
		case ROT_AXIS_X:
			gLight.Position = XMVectorSet(
				0.0f,
				10.0f * cosf(XMConvertToRadians(theta)),
			   -10.0f * sinf(XMConvertToRadians(theta)),
				1.0f
			);
			break;
		case ROT_AXIS_Y:
			gLight.Position = XMVectorSet(
				10.0f * sinf(XMConvertToRadians(theta)),
				0.0f,
			   -10.0f * cosf(XMConvertToRadians(theta)),
				1.0f
			);
			break;
		case ROT_AXIS_Z:
			gLight.Position = XMVectorSet(
				10.0f * cosf(XMConvertToRadians(theta)),
			    10.0f * sinf(XMConvertToRadians(theta)),
			   -10.0f,
				1.0f
			);
			break;
		}
		constantBuffer.LightPosition = gLight.Position;

		// [1, 1] material: emerald
		d3d11Viewport.TopLeftX = 0.0f;
		d3d11Viewport.TopLeftY = 0.0f;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.0215f, 0.1745f, 0.0215f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.07568f, 0.61424f, 0.07568f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.633f, 0.727811f, 0.633f, 1.0f);
		constantBuffer.MaterialShininess = 0.6f * 128.0f;
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [2, 1] material: jade
		d3d11Viewport.TopLeftX = 0.0f;
		d3d11Viewport.TopLeftY = windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.135f, 0.2225f, 0.1575f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.54f, 0.89f, 0.63f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.316228f, 0.316228f, 0.316228f, 1.0f);
		constantBuffer.MaterialShininess = 0.1f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [3, 1] material: obsidian
		d3d11Viewport.TopLeftX = 0.0f;
		d3d11Viewport.TopLeftY = 2 * windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.05375f, 0.05f, 0.06625f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.18275f, 0.17f, 0.22525f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.332741f, 0.328634f, 0.346435f, 1.0f);
		constantBuffer.MaterialShininess = 0.3f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [4, 1] material: pearl
		d3d11Viewport.TopLeftX = 0.0f;
		d3d11Viewport.TopLeftY = 3 * windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.25f, 0.20725f, 0.20725f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(1.0f, 0.829f, 0.829f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.296648f, 0.296648f, 0.296648f, 1.0f);
		constantBuffer.MaterialShininess = 0.3f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [5, 1] material: ruby
		d3d11Viewport.TopLeftX = 0.0f;
		d3d11Viewport.TopLeftY = 4 * windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.1745f, 0.01175f, 0.01175f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.61424f, 0.04136f, 0.04136f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.727811f, 0.727811f, 0.727811f, 1.0f);
		constantBuffer.MaterialShininess = 0.6f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [6, 1] material: turquoise
		d3d11Viewport.TopLeftX = 0.0f;
		d3d11Viewport.TopLeftY = 5 * windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.1f, 0.18725f, 0.1745f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.396f, 0.74151f, 0.69102f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.297254f, 0.30829f, 0.306678f, 1.0f);
		constantBuffer.MaterialShininess = 0.1f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [1, 2] material: brass
		d3d11Viewport.TopLeftX = windowWidth / 4;
		d3d11Viewport.TopLeftY = 0.0f;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.329412f, 0.223529f, 0.027451f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.780392f, 0.568627f, 0.113725f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.992157f, 0.941176f, 0.807843f, 1.0f);
		constantBuffer.MaterialShininess = 0.21794872f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [2, 2] material: bronze
		d3d11Viewport.TopLeftX = windowWidth / 4;
		d3d11Viewport.TopLeftY = windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.2125f, 0.1275f, 0.054f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.714f, 0.4284f, 0.18144f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.393548f, 0.271906f, 0.166721f, 1.0f);
		constantBuffer.MaterialShininess = 0.2f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [3, 2] material: chrome
		d3d11Viewport.TopLeftX = windowWidth / 4;
		d3d11Viewport.TopLeftY = 2 * windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.25f, 0.25f, 0.25f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.4f, 0.4f, 0.4f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.774597f, 0.774597f, 0.774597f, 1.0f);
		constantBuffer.MaterialShininess = 0.6f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [4, 2] material: copper
		d3d11Viewport.TopLeftX = windowWidth / 4;
		d3d11Viewport.TopLeftY = 3 * windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.19125f, 0.0735f, 0.0225f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.7038f, 0.27048f, 0.0828f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.256777f, 0.137622f, 0.086014f, 1.0f);
		constantBuffer.MaterialShininess = 0.1f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [5, 2] material: gold
		d3d11Viewport.TopLeftX = windowWidth / 4;
		d3d11Viewport.TopLeftY = 4 * windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.24725f, 0.1995f, 0.0745f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.75164f, 0.60648f, 0.22648f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.628281f, 0.555802f, 0.366065f, 1.0f);
		constantBuffer.MaterialShininess = 0.4f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [6, 2] material: silver
		d3d11Viewport.TopLeftX = windowWidth / 4;
		d3d11Viewport.TopLeftY = 5 * windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.19225f, 0.19225f, 0.19225f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.50754f, 0.50754f, 0.50754f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.508273f, 0.508273f, 0.508273f, 1.0f);
		constantBuffer.MaterialShininess = 0.4f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [1, 3] material: black plastic
		d3d11Viewport.TopLeftX = 2 * windowWidth / 4;
		d3d11Viewport.TopLeftY = 0.0f;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.01f, 0.01f, 0.01f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f);
		constantBuffer.MaterialShininess = 0.25f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [2, 3] material: cyan plastic
		d3d11Viewport.TopLeftX = 2 * windowWidth / 4;
		d3d11Viewport.TopLeftY = windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.0f, 0.1f, 0.06f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.0f, 0.50980392f, 0.50980392f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.50196078f, 0.50196078f, 0.50196078f, 1.0f);
		constantBuffer.MaterialShininess = 0.25f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [3, 3] material: green plastic
		d3d11Viewport.TopLeftX = 2 * windowWidth / 4;
		d3d11Viewport.TopLeftY = 2 * windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.0f, 0.1f, 0.06f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.1f, 0.35f, 0.1f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.45f, 0.55f, 0.45f, 1.0f);
		constantBuffer.MaterialShininess = 0.25f * 128.0f;	
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [4, 3] material: red plastic
		d3d11Viewport.TopLeftX = 2 * windowWidth / 4;
		d3d11Viewport.TopLeftY = 3 * windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.5f, 0.0f, 0.0f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.7f, 0.6f, 0.6f, 1.0f);
		constantBuffer.MaterialShininess = 0.25f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [5, 3] material: white plastic
		d3d11Viewport.TopLeftX = 2 * windowWidth / 4;
		d3d11Viewport.TopLeftY = 4 * windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.55f, 0.55f, 0.55f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.7f, 0.7f, 0.7f, 1.0f);
		constantBuffer.MaterialShininess = 0.25f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [6, 3] material: yellow plastic
		d3d11Viewport.TopLeftX = 2 * windowWidth / 4;
		d3d11Viewport.TopLeftY = 5 * windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.0f, 0.0f, 0.0f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.5f, 0.5f, 0.0f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.6f, 0.6f, 0.5f, 1.0f);
		constantBuffer.MaterialShininess = 0.25f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [1, 4] material: black rubber
		d3d11Viewport.TopLeftX = 3 * windowWidth / 4;
		d3d11Viewport.TopLeftY = 0.0f;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.02f, 0.02f, 0.02f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.01f, 0.01f, 0.01f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.4f, 0.4f, 0.4f, 1.0f);
		constantBuffer.MaterialShininess = 0.078125f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [2, 4] material: black rubber
		d3d11Viewport.TopLeftX = 3 * windowWidth / 4;
		d3d11Viewport.TopLeftY = windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.0f, 0.05f, 0.05f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.4f, 0.5f, 0.5f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.04f, 0.7f, 0.7f, 1.0f);
		constantBuffer.MaterialShininess = 0.078125f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [3, 4] material: green rubber
		d3d11Viewport.TopLeftX = 3 * windowWidth / 4;
		d3d11Viewport.TopLeftY = 2 * windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.0f, 0.05f, 0.05f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.4f, 0.5f, 0.4f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.04f, 0.7f, 0.04f, 1.0f);
		constantBuffer.MaterialShininess = 0.078125f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [4, 4] material: red rubber
		d3d11Viewport.TopLeftX = 3 * windowWidth / 4;
		d3d11Viewport.TopLeftY = 3 * windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.05f, 0.0f, 0.0f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.5f, 0.4f, 0.4f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.7f, 0.04f, 0.04f, 1.0f);
		constantBuffer.MaterialShininess = 0.078125f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [5, 4] material: red rubber
		d3d11Viewport.TopLeftX = 3 * windowWidth / 4;
		d3d11Viewport.TopLeftY = 4 * windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.05f, 0.05f, 0.05f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.5f, 0.5f, 0.5f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.7f, 0.7f, 0.7f, 1.0f);
		constantBuffer.MaterialShininess = 0.078125f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);

		// [6, 4] material: red rubber
		d3d11Viewport.TopLeftX = 3 * windowWidth / 4;
		d3d11Viewport.TopLeftY = 5 * windowHeight / 6;
		gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
		constantBuffer.MaterialKa = XMVectorSet(0.05f, 0.05f, 0.0f, 1.0f);
		constantBuffer.MaterialKd = XMVectorSet(0.5f, 0.5f, 0.4f, 1.0f);
		constantBuffer.MaterialKs = XMVectorSet(0.7f, 0.7f, 0.04f, 1.0f);
		constantBuffer.MaterialShininess = 0.078125f * 128.0f;		
		gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
		gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);
	}
	else
	{
		constantBuffer.LightIsEnabled = 0U;

		for (int x = 0; x < windowWidth; x += windowWidth / 4)
		{
			for (int y = 0; y < windowHeight; y += windowHeight / 6)
			{
				d3d11Viewport.TopLeftX = x;
				d3d11Viewport.TopLeftY = y;
				gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);
				gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);
				gpID3D11DeviceContext->DrawIndexed(gNumIndices, 0U, 0);
			}
		}
	}
}
