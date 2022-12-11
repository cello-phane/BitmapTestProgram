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

//Vector for the x,y
//The type of vec can be float,int, or double
template<typename T>
struct Vec2
{
  Vec2() : x(T(0)), y(T(0)) {}
  Vec2(const T &x_) : x(x_), y(x_) {}
  Vec2(T x_, T y_) : x(x_), y(y_) {}

  T getX() const
  {
    return x;
  }
  void setX(T x_)
  {
    x = x_;
  }
  T getY() const
  {
    return y;
  }
  void setY(T y_)
  {
    y = y_;
  }
private:
  T x, y;
};
// Draws a // pixel at X, Y (from top left corner)
void DrawPixel_s(int X, int Y, u32& Color) {
  u32 *Pixel = (u32 *)BitmapMemory;
  Pixel += Y * BitmapWidth + X;
  *Pixel = Color;
}
// Draws a // pixel at X, Y (from top left corner)
void DrawPixel(int X, int Y, u32& Color) {
  u32 *Pixel = (u32 *)BitmapMemory;
  Pixel += Y * BitmapWidth + X;
  *Pixel = Color;
  u32 *Pixel_a = (u32 *)BitmapMemory;
  u32 *Pixel_b = (u32 *)BitmapMemory;
  Pixel_a += (Y) * BitmapWidth + (X+1);
  Pixel_b += (Y) * BitmapWidth + (X-1);
  *Pixel_a = Color;
  *Pixel_b = Color;
}

//The functions for squares and lines below use these coordinate xy
/* xy------       <--- xa, ya
   |      |
   |      |
   |______xy      <--- xb, yb
 */

//Draws the lines parallel and perpendicular from top-left corner and bot-right corner points
// Coordinates passed are the top left xy and bot right xy
template<typename coordType>
void OutlineSquare(Vec2<coordType>& Vec_a, Vec2<coordType>& Vec_b, u32& Color){
  auto xa = Vec_a.getX(), ya = Vec_a.getY(), xb = Vec_b.getX(), yb = Vec_b.getY();
  for(auto x = xb - xa;x < xb;++x){
    DrawPixel(x, yb, Color); //Horiz bottom
    DrawPixel(x, ya, Color); //Horiz top
  }//Points should make parallel lines
  for(auto dx = xb - xa, y = ya; y < yb + 1;++y){
    DrawPixel(dx, y, Color); //Vert left
    DrawPixel(xb, y, Color); //Vert right
  }
}

//Draws pixel points on each corner point(vertex)
template<typename coordType>
void VertexPointSquare(Vec2<coordType>& Vec_a, Vec2<coordType>& Vec_b, u32& Color){
  auto xa = Vec_a.getX(), ya = Vec_a.getY(), xb = Vec_b.getX(), yb = Vec_b.getY();
  DrawPixel(xb - xa, ya, Color);      //top left point
  DrawPixel(xb, ya, Color);           //top right point
  DrawPixel(xb - xa, yb, Color);      //bot left point
  DrawPixel(xb, yb, Color);           //bot right point
}
//Fills the area inside the coordinates
template<typename coordType>
void FillSquare(Vec2<coordType>& Vec_a, Vec2<coordType>& Vec_b, u32& Color){
  auto xa = Vec_a.getX(), ya = Vec_a.getY(), xb = Vec_b.getX(), yb = Vec_b.getY();
  //Draw Horiz Parallel lines and  Vert Parallel lines
  for(auto x = xb - xa;x < xb;++x){
    for(auto y = ya;y <= yb;++y){
      DrawPixel(x, y, Color); //Horiz
    }
  }
}
template<typename coordType>
void DiagonalLine(Vec2<coordType>& Vec_a, Vec2<coordType>& Vec_b,
                  u32& Color, const std::string& dir){
  auto xa = Vec_a.getX(), ya = Vec_a.getY(), xb = Vec_b.getX(), yb = Vec_b.getY();
  if(dir == "back" || dir == "downward"){
    /* \  */
    for(auto dx=xb,dy=ya;dx>=xa && dy<=yb;--dx,++dy){
      DrawPixel(dx, dy, Color);
    }
  }
  if(dir == "front" || dir == "forward" || dir == "upward"){
    /* / */
    for(auto dx=xa,dy=ya;dx<=xb && dy<=yb;++dx,++dy){
      DrawPixel(dx, dy, Color);
    }
  }
}
//xa,ya and xb,yb are the top point and bottom point, that are adjacent to the right angle
template<typename coordType>
void OutlineRightTriangle(Vec2<coordType>& Vec_a, Vec2<coordType>& Vec_b,
                          u32& Color, bool vflip=false, bool hflip=false){
  auto xa = Vec_a.getX(), ya = Vec_a.getY(), xb = Vec_b.getX(), yb = Vec_b.getY();
  if((vflip == true && hflip == false) || (vflip == false && hflip == true)) {
    DiagonalLine<coordType>(Vec_a, Vec_b, Color, "back");/* /| */
  }
  else{
    DiagonalLine<coordType>(Vec_a, Vec_b, Color, "front");/* |\ */
  }
 for(auto dx=xa, dy=ya;dx < xb && dy < yb;++dy,++dx){
    if(vflip == false){
      DrawPixel(dx, yb, Color);//Horiz bot
    }
    else{
      DrawPixel(dx, ya, Color);//Horiz top
    }
    if(hflip == false){
      DrawPixel(xa, dy, Color);//Vert left
    }
    else{
      DrawPixel(xb, dy, Color);//Vert right
    }
  }
}
template<typename coordType>
void OutlineEquilTriangle(Vec2<coordType>& Vec_a, Vec2<coordType>& Vec_b,
                          u32& Color, bool vflip=false){
  auto xa = Vec_a.getX(), ya = Vec_a.getY(), xb = Vec_b.getX(), yb = Vec_b.getY();
  DiagonalLine<coordType>(Vec_a, Vec_b, Color, "back");
  Vec2<coordType> Vec_c {xa+(xb-xa), ya};
  Vec2<coordType> Vec_d {xb+(xb-xa), yb};
  DiagonalLine<coordType>(Vec_c, Vec_d, Color, "front");
  for(auto dx=xb-(xb-xa);dx < xb+(yb-ya);++dx){
    DrawPixel(dx, yb, Color);//Horiz bot
  }
}
template<typename coordType>
void OutlineParallelogram(Vec2<coordType>& Vec_a, Vec2<coordType>& Vec_b,
                          u32& Color, const std::string& dir="right"){
  auto xa = Vec_a.getX(), ya = Vec_a.getY(), xb = Vec_b.getX(), yb = Vec_b.getY();
  Vec2<coordType> Vec_c {xb,ya};
  Vec2<coordType> Vec_d {xa+xb,yb};
  if (dir == "right"){
    DiagonalLine<coordType>(Vec_a, Vec_b, Color, "back");
    DiagonalLine<coordType>(Vec_c, Vec_d, Color, "back");
  }
  else if (dir == "left"){
    DiagonalLine<coordType>(Vec_a, Vec_b, Color, "front");
    DiagonalLine<coordType>(Vec_c, Vec_d, Color, "front");
  }
  for(auto dx=xb-xa, dx_=xb+xa;dx < xb;++dx,--dx_){
    if(dir == "left"){
      DrawPixel(dx, ya, Color);//Horiz bot
      DrawPixel(dx_, yb, Color);//Horiz top
    }
    else{
      DrawPixel(dx, yb, Color);//Horiz bot
      DrawPixel(dx+(xb-xa), yb-(xb-xa), Color);//Horiz top
    }
  }
}
void ClearScreen(u32 Color) {
    u32 *Pixel = (u32 *)BitmapMemory;
    for(int Index = 0;
        Index < BitmapWidth * BitmapHeight;
        ++Index) { *Pixel++ = Color; }
}
//Fills the area inside the coordinates
template<typename coordType>
void OutlineGrid(Vec2<coordType>& Vec_a, Vec2<coordType>& Vec_b, int&& cell_size, u32& Color){
  auto xa = Vec_a.getX(), ya = Vec_a.getY(), xb = Vec_b.getX(), yb = Vec_b.getY();
  for(auto x = xa;x < xb;++x){
    for(auto dy=ya;dy < yb;dy=dy+cell_size){
        DrawPixel_s(x, dy, Color);
    }
  }
  for(auto y = ya;y < yb;++y){
    for(auto dx=xa;dx < xb;dx=dx+cell_size){
        DrawPixel_s(dx, y, Color);
    }
  }
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

    HWND Window = CreateWindowEx(0, ClassName, L"Program",WS_OVERLAPPEDWINDOW|WS_VISIBLE,
                                 CW_USEDEFAULT, CW_USEDEFAULT,CW_USEDEFAULT, CW_USEDEFAULT,0, 0, Instance, 0);
    if(!Window) {
        MessageBox(0, L"CreateWindowEx failed", 0, 0);
        return GetLastError();
    }
    // Get client area dimensions
    RECT ClientRect;
    GetClientRect(Window, &ClientRect);
    ClientWidth = ClientRect.right - ClientRect.left + 1052;
    ClientHeight = ClientRect.bottom - ClientRect.top + 352;

    BitmapWidth = ClientWidth;
    BitmapHeight = ClientHeight;

    // Allocate memory for the bitmap
    int BytesPerPixel = 4;

    BitmapMemory = VirtualAlloc(0,BitmapWidth * BitmapHeight * BytesPerPixel,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);

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

    //Initialize a vector for `top` coordinate and `bot` coordinate
    typedef Vec2<int> Vec2int;//using integers for coordinates
    Vec2int top = {BitmapWidth/4, BitmapHeight/6};
    Vec2int bot = {BitmapWidth/2, top.getX() + top.getY()};
    Vec2int max_top{18, 17};
    Vec2int max_bot{1900, 988};//w, h

    //top = xa,ya and bot = ya,yb // these coordinates are top left and bottom right of the geometric object
    //  coordinates have these properties:
    //The points would result in equal sides if these are true:
    // xb = xa * 2
    // xa - yb + ya = 0
    //--Other properties:
    // yb - ya = xa
    // xa + ya = yb
    // xb - xa + ya = yb
    // xb - yb + ya = xa
    // xa - ya + yb = xb
    // xa - xb + yb = ya
   [[maybe_unused]] u32 darkgrey = 0x1F2022;
   [[maybe_unused]] u32 offwhite = 0xF2E9EA;//
   [[maybe_unused]] u32 purple = 0x5D3754;
   [[maybe_unused]] u32 green = 0xAADB1E;
   [[maybe_unused]] u32 cyan = 0xBDDED4;
   [[maybe_unused]] u32 cyan_light = 0xC8EBE0;
    while(Running) {
        MSG Message;
        while(PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE)) {
            if(Message.message == WM_QUIT) Running = false;
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        ClearScreen(offwhite);
        /*        Function Call        */
        OutlineGrid      (max_top, max_bot, 20, cyan_light);
        // FillSquare       (top, bot, green);// "|#|"        // OutlineSquare(top, bot, green);// "|_|"
        // VertexPointSquare(top, bot, purple);// ": :"
        // DiagonalLine      (top, bot, purple, "front"); // "\" //
        // DiagonalLine      (top, bot, purple, "back"); //  "/" //

        // For the right triangle function:
         // The 2 bool parameters after Color are vflip, hflip
         // And the default false, false is a triangle with a right angle at the left side of the base
        // OutlineRightTriangle(top, bot, purple, false, false);/*default ->   "|\"  */
        // OutlineRightTriangle(top, bot, purple, true, false); /*        ->   "|/"  */
        // OutlineRightTriangle(top, bot, purple, false, true); /*        ->   "/|"  */
        // OutlineRightTriangle(top, bot, purple, true, true);  /*        ->   "\|"  */
        OutlineParallelogram(top, bot, purple, "left");/*              ->   "\\"  */
        OutlineParallelogram(top, bot, purple, "right");/*             ->   "//"  */
        // OutlineEquilTriangle(top, bot, purple, false);/*               ->   "/\"  */
        StretchDIBits(DeviceContext,0, 0,BitmapWidth, BitmapHeight,0, 0,ClientWidth, ClientHeight,
                      BitmapMemory, &BitmapInfo,DIB_RGB_COLORS, SRCCOPY);
    }

    return 0;
}
