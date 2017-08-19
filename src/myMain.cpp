#include <windows.h>
#include <stdint.h>

#define internal static
#define global_variable static
#define local_persist static

global_variable bool Running;
global_variable BITMAPINFO BitmapInfo;
global_variable void *BitmapMemory;
global_variable HBITMAP BitmapHandle;
global_variable HDC BitmapDeviceContext;
global_variable int BitmapWidth;
global_variable int BitmapHeight;

internal void
win32ResizeDIBSection(int width, int height)
{
    if(BitmapMemory)
	VirtualFree(BitmapMemory, 0, MEM_RELEASE);

    BitmapWidth = width;
    BitmapHeight = height;
    
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    /*
    HDC DeviceContext = CreateCompatibleDC(0);
    BitmapHandle =  CreateDIBSection(
	DeviceContext,
	&BitmapInfo,
	DIB_RGB_COLORS,
	&BitmapMemory,
	NULL,
	NULL
	);
    ReleaseDC(0, DeviceContext);
    */

    int BytesPerPixel = 4;
    int BitmapMemorySize = (BitmapWidth*BitmapHeight)*BytesPerPixel;
    BitmapMemory = VirtualAlloc(0, BitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);

    int Pitch = BitmapWidth*BytesPerPixel;
    uint8_t *Row = (uint8_t *)BitmapMemory;
    for(int Y = 0; Y < BitmapHeight; Y++)
    {
	uint8_t *Pixel = (uint8_t *)Row;
	for(int X = 0; X < BitmapWidth; X++)
	{
	    // pixel mem: BB GG RR xx
	    *Pixel = (uint8_t)X;
	    ++Pixel;

	    *Pixel = (uint8_t)Y;
	    ++Pixel;

	    *Pixel = 0;
	    ++Pixel;

	    *Pixel = 0;
	    ++Pixel;

	}
	Row += Pitch;
    }
}


internal void
win32UpdateWindow(HDC DeviceContext, RECT *WindowRect, int X, int Y, int width, int height)
{

    int WindowWidth = WindowRect->right - WindowRect->left;
    int WindowHeight = WindowRect->bottom - WindowRect->top;
    StretchDIBits(
	DeviceContext,
/*
	X, Y, width, height,
	X, Y, width, height,
*/
	0, 0, BitmapWidth, BitmapHeight,
	0, 0, WindowWidth, WindowHeight,
	BitmapMemory,
	&BitmapInfo,
	DIB_RGB_COLORS,
	SRCCOPY
	);

    
}

LRESULT CALLBACK win32WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    LRESULT Result = 0;
    switch(msg)
    {
    case WM_SIZE:
    {
	RECT ClientRect;
	GetClientRect(hwnd, &ClientRect);
	int width = ClientRect.right - ClientRect.left;
	int height = ClientRect.bottom - ClientRect.top;
	win32ResizeDIBSection(width, height);
	break;
    }
    case WM_PAINT:
    {
	//TODO: Begin paint
	PAINTSTRUCT Paint;
	HDC DeviceContext = BeginPaint(hwnd, &Paint);
	int X = Paint.rcPaint.left;
	int Y = Paint.rcPaint.top;
	int width = Paint.rcPaint.right - Paint.rcPaint.left;
	int height = Paint.rcPaint.bottom - Paint.rcPaint.top;
	RECT ClientRect;
	GetClientRect(hwnd, &ClientRect);

	win32UpdateWindow(DeviceContext, &ClientRect, X, Y, width, height);
	//TODO: End paint
	break;
    }
    case WM_CLOSE:
    {
	Running = false;
	DestroyWindow(hwnd);
        break;
    }
    case WM_DESTROY:
    {
	Running = false;
	PostQuitMessage(0);
        break;
    }
    default:
    {
	Result = DefWindowProc(hwnd, msg, wParam, lParam);
	break;
    }
    }
    return Result;
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
		   LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc = {};
    HWND hwnd;
    MSG Msg;

    wc.style         = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = win32WndProc;
    wc.cbClsExtra    = 0;
    wc.cbWndExtra    = 0;
    wc.hInstance     = hInstance;
    wc.hIcon         = LoadIcon(NULL, IDI_APPLICATION);
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW+1);
    wc.lpszMenuName  = NULL;
    wc.lpszClassName = "Main Window Class";


    if(RegisterClass(&wc))
    {
	hwnd = CreateWindowEx(
	    0,
	    wc.lpszClassName,
	    "Main Window",
	    WS_OVERLAPPEDWINDOW | WS_VISIBLE,
	    CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
	    NULL, NULL, hInstance, NULL);

	if(hwnd)
	{
	    Running = true;
	    // Main Game Loop
	    while(Running)
	    {      
		BOOL isMessage = GetMessage(&Msg, NULL, 0, 0);
		if(isMessage > 0)
		{
		    TranslateMessage(&Msg);
		    DispatchMessage(&Msg);
		}
	    }
	}
	else
	{
	    //TODO: Error creating window
	}
    }
    else
    {
	//TODO: Error registering class
    }
    return 0;
}

