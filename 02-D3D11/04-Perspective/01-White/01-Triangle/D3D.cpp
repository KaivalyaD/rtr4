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

// linking D3D libraries
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "d3dcompiler.lib")

// macros
#define WIN_WIDTH 800
#define WIN_HEIGHT 600

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// global variable declarations
FILE *gpLog = NULL;
HWND ghwnd = NULL;
BOOL gbFullScreen = FALSE;
BOOL gbActiveWindow = FALSE;

enum {
	KVD_ATTRIBUTE_POSITION = 0,
	KVD_ATTRIBUTE_COLOR,
	KVD_ATTRIBUTE_NORMAL,
	KVD_ATTRIBUTE_TEXTURE0
};

// D3D11 global variables
IDXGISwapChain *gpIDXGISwapChain = NULL;
ID3D11Device *gpID3D11Device = NULL;
ID3D11DeviceContext *gpID3D11DeviceContext = NULL;
ID3D11RenderTargetView *gpID3D11RenderTargetView = NULL;
float clearColor[4];

// shaders
ID3D11VertexShader *gpID3D11VertexShader = NULL;
ID3D11PixelShader *gpID3D11PixelShader = NULL;

// input layout: specifies 'in', 'out', 'layout', etc. (of OpenGL) in shaders
ID3D11InputLayout *gpID3D11InputLayout = NULL;

// buffers
ID3D11Buffer *gpID3D11Buffer_PositionBuffer = NULL;
ID3D11Buffer *gpID3D11Buffer_ConstantBuffer = NULL;  // uniform buffer in OpenGL

// CPU-side uniform storage
struct CBuffer
{
	XMMATRIX WorldViewProjection;  // model-view-projection matrix
};

// projection matrix
XMMATRIX PerspectiveProjectionMatrix;

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
		default:
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
	/**
	 * POSITION: shall be defined in the member: 'SemanticName' of input layout;
	 * SV_POSITION: similar to specifying main() returns a float4 to be considered like gl_Position in OpenGL
	 * SV_: Shader Variable
	 */
	const char *vertexShaderSourceCode =
		"cbuffer constantBuffer\n" \
		"{\n" \
		"	float4x4 worldViewProjectionMatrix;\n" \
		"}\n" \
		"\n" \
		"float4 main(float4 pos: POSITION): SV_POSITION\n" \
		"{\n" \
		"	float4 position;\n" \
		"	position = mul(worldViewProjectionMatrix, pos);\n" \
		"	return position;\n" \
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
		"float4 main(void): SV_TARGET\n" \
		"{\n" \
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

	// configure input layout for attribute 'POSITION' (functions like glVertexAttribPointer())
	D3D11_INPUT_ELEMENT_DESC d3d11InputElementDescriptor;
	ZeroMemory((void *)&d3d11InputElementDescriptor, sizeof(d3d11InputElementDescriptor));
	d3d11InputElementDescriptor.SemanticName = "POSITION";
	d3d11InputElementDescriptor.Format = DXGI_FORMAT_R32G32B32_FLOAT;  // pass as a float3
	d3d11InputElementDescriptor.InputSlot = KVD_ATTRIBUTE_POSITION;
	d3d11InputElementDescriptor.InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;

	// create input layout
	hr = gpID3D11Device->CreateInputLayout(
		&d3d11InputElementDescriptor,
		1,
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
	const float vertexPositions[] = {
		0.0f,  1.0f, 0.0f,
	    1.0f, -1.0f, 0.0f,
	   -1.0f, -1.0f, 0.0f
	};

	// positions buffer
	// configure
	D3D11_BUFFER_DESC d3d11BufferDescriptor;
	ZeroMemory((void *)&d3d11BufferDescriptor, sizeof(d3d11BufferDescriptor));
	d3d11BufferDescriptor.Usage = D3D11_USAGE_DEFAULT;  // GPU Readable/Writable
	d3d11BufferDescriptor.ByteWidth = sizeof(float) * _ARRAYSIZE(vertexPositions);
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

	// constant buffer
	// configure
	ZeroMemory((void *)&d3d11BufferDescriptor, sizeof(d3d11BufferDescriptor));
	d3d11BufferDescriptor.Usage = D3D11_USAGE_DEFAULT;  // GPU Readable/Writable
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

	// set into vertex shader stage of pipeline
	gpID3D11DeviceContext->VSSetConstantBuffers(0, 1, &gpID3D11Buffer_ConstantBuffer);

	// initialize the clear color
	clearColor[0] = 0.0f;
	clearColor[1] = 0.0f;
	clearColor[2] = 1.0f;  // blue
	clearColor[3] = 1.0f;

	PerspectiveProjectionMatrix = XMMatrixIdentity();

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
		fprintf(gpLog, "resize: gpIDXGISwapChain::ResizeBuffers failed with error code 0x%lx\n", hr);
		fclose(gpLog);
		return hr;
	}

	// put the resized buffer from swap chain into a dummy texture
	ID3D11Texture2D *pID3D11Texture2D_BackBuffer = NULL;  // front buffer is already given by D3D
	hr = gpIDXGISwapChain->GetBuffer(0U, __uuidof(ID3D11Texture2D), (LPVOID *)&pID3D11Texture2D_BackBuffer);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "resize: gpIDXGISwapChain::GetBuffer failed with error code 0x%lx\n", hr);
		fclose(gpLog);
		return hr;
	}

	// create a new RTV using the new buffer
	hr = gpID3D11Device->CreateRenderTargetView(pID3D11Texture2D_BackBuffer, NULL, &gpID3D11RenderTargetView);
	if (FAILED(hr))
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "resize: gpID3D11Device::CreateRenderTargetView failed with error code 0x%lx\n", hr);
		fclose(gpLog);
		return hr;
	}

	// release the dummy texture
	pID3D11Texture2D_BackBuffer->Release();
	pID3D11Texture2D_BackBuffer = NULL;

	// set the new RTV in pipeline
	gpID3D11DeviceContext->OMSetRenderTargets(1U, &gpID3D11RenderTargetView, NULL);

	// initialize the viewport
	D3D11_VIEWPORT d3d11Viewport;
	d3d11Viewport.TopLeftX = 0.0f;  // viewport origin is top-left corner
	d3d11Viewport.TopLeftY = 0.0f;
	d3d11Viewport.Width = (FLOAT)width;
	d3d11Viewport.Height = (FLOAT)height;
	d3d11Viewport.MinDepth = 0.0f;
	d3d11Viewport.MaxDepth = 1.0f;

	// set the viewport in pipeline
	gpID3D11DeviceContext->RSSetViewports(1U, &d3d11Viewport);

	// reset projection matrix
	FLOAT aspectRatio = (FLOAT)width / (FLOAT)height;
	PerspectiveProjectionMatrix = XMMatrixPerspectiveFovLH(
		XMConvertToRadians(45.0f),
		aspectRatio,
		0.1f,
		100.0f
	);

	return hr;
}

void display(void)
{
	// code
	// clear RTV
	gpID3D11DeviceContext->ClearRenderTargetView(gpID3D11RenderTargetView, clearColor);

	// set position buffer in input assembly stage of pipeline
	UINT stride = sizeof(float) * 3;
	UINT offset = 0;
	gpID3D11DeviceContext->IASetVertexBuffers(0, 1, &gpID3D11Buffer_PositionBuffer, &stride, &offset);

	// set primitive topology in input assembly stage of pipeline (there are 42 in total)
	gpID3D11DeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// transformations
	XMMATRIX worldMatrix = XMMatrixTranslation(0.0f, 0.0f, 4.0f);
	XMMATRIX viewMatrix = XMMatrixIdentity();
	XMMATRIX wvpMatrix = worldMatrix * viewMatrix * PerspectiveProjectionMatrix;

	// push wvpMatrix into constant buffer
	CBuffer constantBuffer;
	ZeroMemory((void *)&constantBuffer, sizeof(constantBuffer));
	constantBuffer.WorldViewProjection = wvpMatrix;
	gpID3D11DeviceContext->UpdateSubresource(gpID3D11Buffer_ConstantBuffer, 0, NULL, &constantBuffer, 0, 0);

	// draw
	gpID3D11DeviceContext->Draw(3, 0);

	// swap buffers by presenting them
	gpIDXGISwapChain->Present(
		0U,  // don't try to sync with monitor refresh rate as 60 Hz is preset
		0U   // swap among all buffers
	);
}

void update(void)
{
	// code
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
	if (gpID3D11Buffer_ConstantBuffer)
	{
		gpID3D11Buffer_ConstantBuffer->Release();
		gpID3D11Buffer_ConstantBuffer = NULL;
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
