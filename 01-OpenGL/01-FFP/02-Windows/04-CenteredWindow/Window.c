// header files
#include<windows.h>

// macros
#define CX_THISWINDOW 600
#define CY_THISWINDOW 600

// global function declarations
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

// entry-point function
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpszCmdLine, int iCmdShow)
{
	// variable declarations
	WNDCLASSEX wndclass;
	HWND hwnd;
	MSG msg;
	TCHAR szAppName[] = TEXT("MyWindow");
	TCHAR str[255];
	int cxScreen, cyScreen;

	// code
	// initialization of the wndclass structure
	wndclass.cbSize = sizeof(WNDCLASSEX);
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpfnWndProc = WndProc;
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = NULL;
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION);

	// getting width and height of primary display
	cxScreen = GetSystemMetrics(SM_CXSCREEN);
	cyScreen = GetSystemMetrics(SM_CYSCREEN);

	// registering the wndclass
	RegisterClassEx(&wndclass);

	// create the window
	hwnd = CreateWindow(szAppName,
		TEXT("Kaivalya Vishwakumar Deshpande"),
		WS_OVERLAPPED | WS_MAXIMIZEBOX | WS_MINIMIZEBOX | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME,
		(cxScreen / 2) - (CX_THISWINDOW / 2),
		(cyScreen / 2) - (CY_THISWINDOW / 2),
		CX_THISWINDOW,
		CY_THISWINDOW,
		NULL,
		NULL,
		hInstance,
		NULL);
	
	// show the window
	ShowWindow(hwnd, iCmdShow);
	// update the window
	UpdateWindow(hwnd);

	wsprintf(str, TEXT("x = %d\ny = %d"), (cxScreen / 2) - (CX_THISWINDOW / 2), (cyScreen / 2) - (CY_THISWINDOW / 2));
	MessageBox(hwnd, str, TEXT("This window's top-left corner co-ordinates"), MB_OK | MB_ICONINFORMATION);

	// message loop
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

// callback function
LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{
	// code
	switch (iMsg)
	{
	case WM_CREATE:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		break;
	}

	return DefWindowProc(hwnd, iMsg, wParam, lParam);
}
