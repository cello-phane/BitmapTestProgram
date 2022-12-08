#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
// #include <wingdi.h>//Don't include this directly, since windows.h implicitly includes that header
#include <stdint.h>
#include <string>

typedef uint32_t u32;

void* BitmapMemory;

int BitmapWidth;
int BitmapHeight;

int ClientWidth;
int ClientHeight;

template<typename T>
//Vector for the x,y,and (and possibly z coords (not a vec datatype from C++ of course..)
//The type of vec can be float,int, or double
class Vec3
{
public:
  // 3 most basic ways of initializing a vector
  Vec3() : x(T(0)), y(T(0)), z(T(0)) {}
  Vec3(const T &xx) : x(xx), y(xx), z(xx) {}
  Vec3(T xx, T yy, T zz) : x(xx), y(yy), z(zz) {}
  T x, y, z;
};

// Draws a pixel at X, Y (from top left corner)
void DrawPixel(int X, int Y, u32 Color) {
    u32 *Pixel = (u32 *)BitmapMemory;
    Pixel += Y * BitmapWidth + X;
    *Pixel = Color;
}

//The functions for squares and lines below use these coordinate xy
/* xy------       <--- xa, ya
   |      |
   |      |
   |______xy      <--- xb, yb
 */

//Draws the lines parallel and perpendicular from top-left corner and bot-right corner points
// Coordinates passed are the top left xy and bot right xy
void OutlineSquare(int xa, int ya, int xb, int yb, u32 Color){
  for(auto x = xb - xa;x < xb;x++){
    DrawPixel(x, yb, Color); //Horiz bottom
    DrawPixel(x, ya, Color); //Horiz top
  }//Points should make parallel lines
  for(auto dx = xb - xa, y = ya; y < yb + 1;y++){
    DrawPixel(dx, y, Color); //Vert left
    DrawPixel(xb, y, Color); //Vert right
  }
}

//Draws pixel points on each corner point(vertex)
void VertexPointSquare(int xa, int ya, int xb, int yb, u32 Color){
  DrawPixel(xb - xa, ya, Color);      //top left point
  DrawPixel(xb, ya, Color);           //top right point
  DrawPixel(xb - xa, yb, Color);      //bot left point
  DrawPixel(xb, yb, Color);           //bot right point
}
//Fills the area inside the coordinates
void FillSquare(int xa, int ya, int xb, int yb, u32 Color){
  //Draw Horiz Parallel lines and  Vert Parallel lines
  for(auto x = xb - xa;x < xb;x++){
    for(auto y = ya;y <= yb;y++){
      DrawPixel(x, y, Color); //Horiz
    }
  }
}
void DiagnolLine(int xa, int ya, int xb, int yb, u32 Color, const std::string& dir){
  if(dir == "back"){
    /* \  */
    for(auto dx=xa,dy=ya;dx<=xb && dy<=yb;dx++,dy++){
      DrawPixel(dx, dy, Color);
    }
  }
  if(dir == "front" || dir == "forward"){
    /* / */
    for(auto dx=xb,dy=ya;dx>=xa && dy<=yb;dx--,dy++){
      DrawPixel(dx, dy, Color);
    }
  }
}
//xa,ya and xb,yb are the top point and bottom point, that are adjacent to the right angle
void OutlineRightTriangle(int xa, int ya, int xb, int yb, u32 Color, bool vflip=false, bool hflip=false){
  if((hflip == false && vflip == false) || (hflip == true && vflip == true)) {
    DiagnolLine(xa, ya, xb, yb, Color, "back");/*makes 90 degree angle at the right of the base /| */
  }
  else{
    DiagnolLine(xa, ya, xb, yb, Color, "front");/*makes 90 degree angle at the left the of basef |\ */
  }
  for(auto dx=xa, dy=ya;dx < xb && dy < yb;dy++,dx++){
    if(hflip == false){
      DrawPixel(xa, dy, Color);//Vert left
      if(vflip == false){
        DrawPixel(dx, yb, Color);//Horiz bot
      }
      else{
        DrawPixel(dx, ya, Color);//Horiz top
      }
    }
    else{//hflip==true
      DrawPixel(xb, dy, Color);//Vert right
      if(vflip == true){
        DrawPixel(dx, ya, Color);//Horiz top
      }
      else{
        DrawPixel(dx, yb, Color);//Horiz bot
      }
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
    // WindowClass.hCursor = LoadCursor(0, IDC_CROSS);

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
    ClientWidth = ClientRect.right - ClientRect.left + 600;
    ClientHeight = ClientRect.bottom - ClientRect.top + 350;

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

    //Initialize a vector `a`
    typedef Vec3<int> Vec3int;
    Vec3int a;

    while(Running) {
        MSG Message;
        while(PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE)) {
            if(Message.message == WM_QUIT) Running = false;
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        ClearScreen(0x333333);
        /*        Function Calls        */
        FillSquare       (468, 132, 936, 600, 0x5D3754);// |#|
        // OutlineSquare    (468, 132, 936, 600, 0xAADB1E);// |_|
        // VertexPointSquare(468, 132, 936, 600, 0xFFFFFF);// : :
        // DiagnolLine      (468, 132, 936, 600, 0xAADB1E, "front"); // \ //
        // DiagnolLine      (468, 132, 936, 600, 0xAADB1E, "back"); //  / //

        //For the right triangle function:
        //  The 2 bool parameters after Color are vflip, hflip
        //  And the default false, false is a triangle with a right angle at the left side of the base
        OutlineRightTriangle(468,132,936,600, 0xAADB1E, false, false);/*default ->   |\  */
        OutlineRightTriangle(468,132,936,600, 0xAADB1E, true, false); /*        ->   \|  */
        OutlineRightTriangle(468,132,936,600, 0xAADB1E, false, true); /*        ->   /|  */
        OutlineRightTriangle(468,132,936,600, 0xAADB1E, true, true);  /*        ->   |/  */

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
