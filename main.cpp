#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
// #include <wingdi.h>
#include <stdint.h>

typedef uint32_t u32;

void* BitmapMemory;

int BitmapWidth;
int BitmapHeight;

int ClientWidth;
int ClientHeight;

// Draws a pixel at X, Y (from top left corner)
void DrawPixel(int X, int Y, u32 Color) {
    u32 *Pixel = (u32 *)BitmapMemory;
    Pixel += Y * BitmapWidth + X;
    *Pixel = Color;
}

//The functions below use these coordinates xy
/* xy------       <--- tleft_x, tleft_y
   |      |
   |      |
   |______xy      <--- bright_x, bright_y
 */

//Draws the lines parallel and perpendicular from top-left corner and bot-right corner points
// Coordinates passed are the top left xy and bot right xy
//OutlineSquare(120,220,220,320, 0xFFFFFF);
void OutlineSquare(int tleft_x, int tleft_y, int bright_x, int bright_y, u32 Color){
  //Draw Horiz Parallel lines and  Vert Parallel lines
  for(auto x = bright_x - tleft_x;x < bright_x;x++){
    DrawPixel(x, bright_y, Color);//Horiz bottom
    DrawPixel(x, tleft_y, Color); //Horiz top
  }
  for(auto x_left = bright_x - tleft_x, y = tleft_y; y < bright_y + 1;y++){
    DrawPixel(x_left, y, Color); //Vert left
    DrawPixel(bright_x, y, Color);//Vert right
  }
}

//Draws pixel points on each corner(vertex)
// Coordinates passed are the top left xy and bot right xy
//VertexPointSquare(120,220,220,320, 0x00FF33); //Example of functin call
void VertexPointSquare(int tleft_x, int tleft_y, int bright_x, int bright_y, u32 Color){
  DrawPixel(bright_x - tleft_x, tleft_y, Color); //top left point
  DrawPixel(bright_x, tleft_y, Color);           //top right point
  DrawPixel(bright_x - tleft_x, bright_y, Color);//bot left point
  DrawPixel(bright_x, bright_y, Color);          //bot right point
}
//Fills the area inside the coordinates
//FillSquare(122,222,218,318, 0x00FF33); //Example of function call
void FillSquare(int tleft_x, int tleft_y, int bright_x, int bright_y, u32 Color){
  //Draw Horiz Parallel lines and  Vert Parallel lines
  for(auto x = bright_x - tleft_x;x < bright_x;x++){
    for(auto i=0,f = bright_y - tleft_y;i <= f;i++){
      DrawPixel(x, tleft_y+i, Color); //Horiz
    }
  }
}

void ClearScreen(u32 Color) {
    u32 *Pixel = (u32 *)BitmapMemory;
    for(int Index = 0;
        Index < BitmapWidth * BitmapHeight;
        ++Index) { *Pixel++ = Color; }
}

LRESULT CALLBACK WindowProc(HWND Window, UINT Message, WPARAM WParam, LPARAM LParam) {
    switch(Message) {
        case WM_KEYDOWN: {
            switch(WParam) { case 'O': { DestroyWindow(Window); }; }
        } break;
        case WM_DESTROY: { PostQuitMessage(0); } break;
        default: { return DefWindowProc(Window, Message, WParam,  LParam); }
    }
    return 0;
}

int WINAPI wWinMain(HINSTANCE Instance, HINSTANCE PrevInstance, PWSTR CmdLine, int CmdShow) {

    WNDCLASS WindowClass = {};
    const wchar_t ClassName[] = L"MyWindowClass";
    WindowClass.lpfnWndProc = WindowProc;
    WindowClass.hInstance = Instance;
    WindowClass.lpszClassName = ClassName;
    WindowClass.hCursor = LoadCursor(0, IDC_CROSS);

    if(!RegisterClass(&WindowClass)) {
        MessageBox(0, L"RegisterClass failed", 0, 0);
        return GetLastError();
    }

    HWND Window = CreateWindowEx(0, ClassName, L"Program",
                                 WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 CW_USEDEFAULT, CW_USEDEFAULT,
                                 0, 0, Instance, 0);

    if(!Window) {
        MessageBox(0, L"CreateWindowEx failed", 0, 0);
        return GetLastError();
    }


    // Get client area dimensions

    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    ClientWidth = ClientRect.right - ClientRect.left;
    ClientHeight = ClientRect.bottom - ClientRect.top;

    BitmapWidth = ClientWidth;
    BitmapHeight = ClientHeight;

    // Allocate memory for the bitmap

    int BytesPerPixel = 4;

    BitmapMemory = VirtualAlloc(0,
                                BitmapWidth * BitmapHeight * BytesPerPixel,
                                MEM_RESERVE|MEM_COMMIT,
                                PAGE_READWRITE
                                );

    // BitmapInfo struct for StretchDIBits

    BITMAPINFO BitmapInfo;
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    // Negative BitmapHeight makes top left as the coordinate system origin for the DrawPixel function, otherwise its bottom left
    BitmapInfo.bmiHeader.biHeight = -BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    HDC DeviceContext = GetDC(Window);

    bool Running = true;

    while(Running) {
        MSG Message;
        while(PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE)) {
            if(Message.message == WM_QUIT) Running = false;
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }

        ClearScreen(0x333333);
        
        //Examples of drawing functions here
        //Fill the sqr with a purple color
        FillSquare(220,220,320,620, 0x5D3754);
        //Outline the sqr with a green color
        OutlineSquare(220,220,320,620, 0xAADB1E);
        //Draw white dot for each corner pixel
        VertexPointSquare(220,220,320,620, 0xFFFFFF);
        
        StretchDIBits(DeviceContext,
                      0, 0,
                      BitmapWidth, BitmapHeight,
                      0, 0,
                      ClientWidth, ClientHeight,
                      BitmapMemory, &BitmapInfo,
                      DIB_RGB_COLORS, SRCCOPY);
    }

    return 0;
}
