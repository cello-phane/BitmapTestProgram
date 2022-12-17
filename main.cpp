#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <cstdlib>
#include <windows.h>
// #include <wingdi.h>//Don't include this directly, since windows.h implicitly includes that header
#include <stdint.h>
#include <string>
#include <iostream>
#include <cassert>
typedef uint32_t u32;

void* BitmapMemory;

int BitmapWidth;
int BitmapHeight;

int ClientWidth;
int ClientHeight;
//Colors in hex
[[maybe_unused]] u32 darkgrey  = 0x1F2022,
                   offwhite    = 0xFCFBF9,
                   purple      = 0x5D3754,
                   purp_lite   = 0x83577D,
                   green       = 0xAADB1E,
                   green_lite  = 0xEEFFEE,
                   cyan        = 0xBDDED4,
                   cyan_lite   = 0xC8EBE0,
                   bluegrey    = 0x3E4C61,
                   blue_lite   = 0x556995,
                   darkblugrey = 0x1E242E,
                   darkgreen   = 0x072C2E,
                   grey        = 0x9E9AA7,
                   grey_lite   = 0xEFF5F5;
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
void DrawPixel(int X, int Y, u32& Color) {
  // auto x_n = SX * X + DX;//
  // auto y_n = SY * Y + DY;//
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
void FillSquare(Vec2<coordType>& Vec_a, Vec2<coordType>& Vec_b, u32& Color, std::string&& dir="right"){
  auto xa = Vec_a.getX(), ya = Vec_a.getY(), xb = Vec_b.getX(), yb = Vec_b.getY();
  //Draw Horiz Parallel lines and  Vert Parallel lines - dir is draw direction
  if (dir=="right"){
    for(auto x = xb - xa;x <= xb;++x){
      for(auto y = ya;y <= yb;++y){
        DrawPixel(x, y, Color); //Right Horiz
      }
    }
  }
  else if (dir=="down"){
    for(auto y = yb - ya;y <= yb;++y){
      for(auto x = xb;x <= xb;++x){
        DrawPixel(x, y, Color); //Down Vert
      }
    }
  }
}
void HLine(int X, int Y, u32& Color) {
  //draws right
  Vec2<int> Vec_dot_a {X, Y};
  Vec2<int> Vec_dot_b {X, Y};
  FillSquare(Vec_dot_a, Vec_dot_b, Color, "right");
}
void VLine(int X, int Y, u32& Color) {
  //draws down
  Vec2<int> Vec_dot_a {X, Y};
  Vec2<int> Vec_dot_b {X, Y};
  FillSquare(Vec_dot_a, Vec_dot_b, Color, "down");
}
template<typename coordType>
void DiagonalLine(Vec2<coordType>& Vec_a, Vec2<coordType>& Vec_b,
                  u32& Color, const std::string& dir){
  auto xa = Vec_a.getX(), ya = Vec_a.getY(), xb = Vec_b.getX(), yb = Vec_b.getY();
  if(dir == "back" || dir == "downward"){
    /* \  */
    for(auto dx=xb,dy=ya;dx>xa && dy<yb;--dx,++dy){
      DrawPixel(dx, dy, Color);
    }
  }
  if(dir == "front" || dir == "forward" || dir == "upward"){
    /* / */
    for(auto dx=xa,dy=ya;dx<xb && dy<yb;++dx,++dy){
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
  auto y_diff = yb-(xb-xa)+1, x_diff = xb-xa-1;
  if (dir == "right"){
    DiagonalLine<coordType>(Vec_a, Vec_b, Color, "back");
    DiagonalLine<coordType>(Vec_c, Vec_d, Color, "back");
  }
  else if (dir == "left"){
    DiagonalLine<coordType>(Vec_a, Vec_b, Color, "front");
    DiagonalLine<coordType>(Vec_c, Vec_d, Color, "front");
  }
  for(auto dx=xb-xa,dx_=xb+xa;dx < xb;++dx,--dx_){
    if(dir == "left"){
      DrawPixel(dx, ya, Color);//Horiz bot
      DrawPixel(dx_, yb, Color);//Horiz top
    }
    else{
      DrawPixel(dx, yb, Color);//Horiz bot
      DrawPixel(dx+x_diff, y_diff, Color);//Horiz top
    }
  }
}
void ClearScreen(u32 Color) {
    u32 *Pixel = (u32 *)BitmapMemory;
    for(int Index = 0;
        Index < BitmapWidth * BitmapHeight;
        ++Index) { *Pixel++ = Color; }
}

template<typename coordType>
void OutlineGrid(Vec2<coordType>& Vec_a, Vec2<coordType>& Vec_b, int& cell_size,int& col_row_cell_n, u32& Color){
  auto xa = Vec_a.getX(), ya = Vec_a.getY(), xb = Vec_b.getX(), yb = Vec_b.getY();
  auto height = Vec_b.getY() - Vec_a.getY();
  auto width = Vec_b.getX() - Vec_a.getX();
  for(auto x = xa;x < xb;++x){
    for(auto dy=ya,alt=0;dy < yb;dy=dy+cell_size,++alt){
      if(alt % col_row_cell_n == 0){
        DrawPixel(x, dy-1, Color);
        DrawPixel(x, dy, Color);
        DrawPixel(x, dy+1, Color);
      }
      else{
        DrawPixel(x, dy, Color);
      }
    }
  }
  for(auto y = ya;y < yb;++y){
    for(auto dx=xa,alt=0;dx < xb;dx=dx+cell_size,alt++){
      if(alt % col_row_cell_n == 0){
        DrawPixel(dx-1, y, Color);
        DrawPixel(dx, y, Color);
        DrawPixel(dx+1, y, Color);
      }
      else{
        DrawPixel(dx, y, Color);
      }
    }
  }
  u32 mid_color = 0xAADB1E;
  auto h_ = std::div(height,cell_size*col_row_cell_n);
  auto w_ = std::div(width,cell_size*col_row_cell_n);
  auto height_normal = static_cast<int>((h_.quot/2) * (cell_size*col_row_cell_n));
  auto width_normal = static_cast<int>((w_.quot/2) * (cell_size*col_row_cell_n));
  auto dh = height-height_normal;
  if(dh > cell_size*col_row_cell_n){
    height = height_normal + cell_size * col_row_cell_n;
  }
  else{
    height = height_normal - cell_size * col_row_cell_n;
  }
  auto dw = width-width_normal;
  if(dw > cell_size*col_row_cell_n){
    width = width_normal + cell_size * col_row_cell_n;
  }
  else{
    width = width_normal - cell_size * col_row_cell_n;
  }
  //assert(height == h_.quot * (cell_size*col_row_cell_n) + h_.rem);
  VLine(width,height,mid_color);
  HLine(width,height,mid_color);
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
    ClientWidth = (ClientRect.right*2) + (ClientRect.left*2)-680;
    ClientHeight = (ClientRect.bottom*2) + (ClientRect.top*2)-340;

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
    BitmapInfo.bmiHeader.biHeight = BitmapHeight;
    BitmapInfo.bmiHeader.biPlanes = 1;
    BitmapInfo.bmiHeader.biBitCount = 32;
    BitmapInfo.bmiHeader.biCompression = BI_RGB;

    HDC DeviceContext = GetDC(Window);

    bool Running = true;

    //Grid Setup/Colors
    int cell_size = 20;
    int col_row_cell_n = 5;
    //padding offset for grid, in px
    //auto grid_pad_left = 1, grid_pad_top = 1;
    typedef Vec2<int> Vec2int;//using integers for coordinates
    //Vec2int grid_top{grid_pad_left, grid_pad_top};
    //Vec2int grid_bot{ClientWidth, ClientHeight};//w, h
    Vec2int grid_top{0,1};
    Vec2int grid_bot{1920,1088};

    while(Running) {
        MSG Message;
        while(PeekMessage(&Message, nullptr, 0, 0, PM_REMOVE)) {
            if(Message.message == WM_QUIT) Running = false;
            TranslateMessage(&Message);
            DispatchMessage(&Message);
        }
        //Setup the grid/background/point of origin
        ClearScreen(offwhite);
        /*        Function Call        */
        OutlineGrid(grid_top, grid_bot, cell_size, col_row_cell_n, grey_lite);

        // HLine(ClientWidth-1, ClientHeight/2, green);
        // VLine(ClientWidth/2, ClientHeight-1, green);
        // DrawPixel(ClientWidth/2, ClientHeight/2,purple);

        //Sample of functions
        //top = xa,ya and bot = ya,yb // these coordinates are top left and bottom right
        Vec2int top = {(BitmapWidth/4)-15, (BitmapHeight/6)+79};
        Vec2int bot = {(BitmapWidth/2)-29, (top.getX() + top.getY())};
        // FillSquare        (top, bot, bluegrey, "right");        /*        ->   "|#|" */
        // OutlineSquare     (top, bot, purp_lite);                /*        ->   "|_|" */
        // VertexPointSquare (top, bot, purp_lite);                /*        ->   ": :" */
        // DiagonalLine      (top, bot, purp_lite, "front");       /*        ->    "\"  */
        // DiagonalLine      (top, bot, purp_lite, "back");        /*        ->    "/"  */
        // OutlineRightTriangle(top, bot, purp_lite, false, false);/*default ->   "|\"  */
        // OutlineRightTriangle(top, bot, purp_lite, true, false); /*        ->   "|/"  */
        // OutlineRightTriangle(top, bot, purp_lite, false, true); /*        ->   "/|"  */
        // OutlineRightTriangle(top, bot, purp_lite, true, true);  /*        ->   "\|"  */
        // OutlineParallelogram(top, bot, purp_lite, "left");      /*        ->   "\\"  */
        // OutlineParallelogram(top, bot, purp_lite, "right");     /*        ->   "//"  */
        OutlineEquilTriangle(top, bot, purp_lite, false);       /*        ->   "/\"  */
        StretchDIBits(DeviceContext,0, 0,BitmapWidth, BitmapHeight,0, 0,ClientWidth, ClientHeight,
                      BitmapMemory, &BitmapInfo,DIB_RGB_COLORS, SRCCOPY);
    }

    return 0;
}
