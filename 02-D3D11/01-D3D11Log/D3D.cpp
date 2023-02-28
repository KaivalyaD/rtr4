// header files
#include<windows.h>
#include<stdlib.h>	// for exit()
#include<stdio.h>	// for file I/O functions
#include<math.h>	// for math()

#include<d3d11.h>   // main Direct3D 11 header
#include<dxgi.h>    // DirectX graphics infrastructure

#include"D3D.h"

// linking D3D libraries
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")

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
	fprintf(gpLog, "fopen_s: opened log file\n", hr);
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
		fprintf(gpLog, "initialize() succeeded\n", hr);
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
	void resize(int, int);

	// code
	switch (iMsg)
	{
	case WM_SETFOCUS:
		gbActiveWindow = TRUE;

		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "window in focus\n");
		fclose(gpLog);
		break;
	case WM_KILLFOCUS:
		gbActiveWindow = FALSE;

		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "window out of focus\n");
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
			fprintf(gpLog, "destroying after receiving esc\n");
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
		resize(LOWORD(lParam), HIWORD(lParam));
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
			fprintf(gpLog, "fullscreen mode on\n");
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
		fprintf(gpLog, "fullscreen mode off\n");
		fclose(gpLog);
	}
}

HRESULT initialize(void)
{
	// function prototypes
	HRESULT PrintD3DInfo(void);

	// variable declarations
	HRESULT hr = S_OK;

	// code
	hr = PrintD3DInfo();

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
		fprintf(gpLog, "CreateDXGIFactory() failed with error code 0x%lx\n", hr);
		fclose(gpLog);
		return hr;
	}
	else
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "CreateDXGIFactory() succeeded\n", hr);
		fclose(gpLog);
	}

	// get IDXGIAdapter (ex. interface to the 0th graphics device connected to the system) from the factory
	if ((hr = pIDXGIFactory->EnumAdapters(0, &pIDXGIAdapter)) != DXGI_ERROR_NOT_FOUND)
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "IDXGIFactory::EnumAdapters() for adapter at ordinal 0 succeeded\n");
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
		fprintf(gpLog, "pIDXGIFactory::EnumAdapters() failed with error code 0x%lx\n", hr);
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

void resize(int width, int height)
{
	// code
	if (height <= 0)
		height = 1;	// to prevent a divide by zero when calculating the width/height ratio
}

void display(void)
{
	// code
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
	if (ghwnd)
	{
		DestroyWindow(ghwnd);	// if unitialize() was not called from WM_DESTROY
		ghwnd = NULL;	// disciplined code: once a handle is invalid, reset to NULL
	}
	if (gpLog)
	{
		fopen_s(&gpLog, "Log.txt", "a+");
		fprintf(gpLog, "fclose: closing log file\n");
		fclose(gpLog);
		gpLog = NULL;
	}
}
