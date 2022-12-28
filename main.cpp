#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
// #include <cstdlib>
// #include <wingdi.h>//Don't include this directly, since windows.h implicitly includes that header
// #include <stdint.h>
#include <string>
// #include <cassert>
// #include <vector>
// #include <tuple>
// #define TAU M_PI*2;//6.283185307179586
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
                   purp_lite   = 0x816282,// 0x83577D,
                   green       = 0xAADB1E,
                   green_lite  = 0xEEFFEE,
                   cyan        = 0xBDDED4,
                   cyan_lite   = 0xC8EBE0,
                   bluegrey    = 0x3E4C61,
                   blue_lite   = 0x556995,
                   darkblugrey = 0x1E242E,
                   darkgreen   = 0x072C2E,
                   grey        = 0x9E9AA7,
                   grey_lite   = 0xEFF5F5,
                   purp_shade  = 0xC3ACC3,
                   purp_shade_lite = 0xEDC3F1,
                   red         = 0xD16563;


//Point - coordinate type T in x,y
template<typename T>
struct Point
{
  Point() : x(T(0)), y(T(0)) {}
  Point(const T &x_) : x(x_), y(x_) {}
  Point(T x_, T y_) : x(x_), y(y_) {}
public:
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
  //This converts the input ClientWindows coordinates to normal cartesian coords
  //TODO: maybe add an option to specify if initial Bitmap Height is negative or positive ( 0,0 is either at bot left or top left )
  void set_ndc(T& offset_x, T& offset_y, T& cell_size){
    x = x >= 0 ? offset_x + x * cell_size : offset_x - abs(x) * cell_size;
    y = y >= 0 ? offset_y + y * cell_size : offset_y - abs(y) * cell_size;
  }
private:
  T x, y;
};


//Grid Setup
float cell_size = 40;//sets pixel count of each cell(grid unit)
float col_row_cell_n = 4;//sets the number of cells inside each col/row
float cell_unit_interval = 0.5;
//padding offset for grid rectangle corners(in pixels)
//offsets are needed or else bitmap program crashes
float grid_pad_left = 1.0, grid_pad_top = 1.0;
using PointInt = Point<int>;//using integers for coordinates
using PointFloat = Point<float>;//using integers for coordinates
PointFloat grid_top{grid_pad_left, grid_pad_top};
PointFloat grid_bot{static_cast<float>(1920), static_cast<float>(1080)};//make a 1080 resolution grid

//For accurate cartesian coordinate view, the axis origin points can be offset for even columns
template<typename T>
T origin_axis (int&& axis_len_, T& cell_size_, T& col_row_cell_n_){
  auto ax_len_offset = static_cast<int>(axis_len_ % static_cast<int>(((cell_size_*cell_unit_interval)*col_row_cell_n_)));
  auto cols_ = static_cast<int>((axis_len_-ax_len_offset)/((cell_size_*cell_unit_interval)*col_row_cell_n_));
  auto fixed_origin = ((cell_size_*cell_unit_interval)*col_row_cell_n_)*static_cast<int>(cols_/2);
  return fixed_origin;
};

//Drawing the axis lines without an offset
//Maybe we can just `clamp` the width/height of each axis line
auto org_x = origin_axis(static_cast<int>(grid_bot.getX()-grid_top.getX()), cell_size, col_row_cell_n);
auto org_y = origin_axis(static_cast<int>(grid_bot.getY()-grid_top.getY()), cell_size, col_row_cell_n);

// Draws a pixel at X, Y (from top left corner)
void DrawPixel(int X, int Y, u32& Color) {
  u32 *Pixel = (u32 *)BitmapMemory;
  Pixel += Y * BitmapWidth + X;
  *Pixel = Color;
}
void DrawPixel(float X, float Y, u32& Color) {
  u32 *Pixel = (u32 *)BitmapMemory;
  Pixel += static_cast<unsigned int>(Y) * BitmapWidth + static_cast<unsigned int>(X);
  *Pixel = Color;
}

//The functions for squares and lines below use these coordinate xy
/* xy-----       <--- xa, ya
   |      |
   |      |
   |______xy      <--- xb, yb
 */

//Draws the lines parallel and perpendicular from top-left corner and bot-right corner points
// Coordinates passed are the top left xy and bot right xy

//Draws pixel points on each corner point(vertex)
template<typename coordType>
void VertexPointSquare(Point<coordType>& Point_a, Point<coordType>& Point_b, u32& Color){
  const auto xa = Point_a.getX(), ya = Point_a.getY(), xb = Point_b.getX(), yb = Point_b.getY();
  DrawPixel(xb - xa, ya, Color);      //top left point
  DrawPixel(xb, ya, Color);           //top right point
  DrawPixel(xb - xa, yb, Color);      //bot left point
  DrawPixel(xb, yb, Color);           //bot right point
}

template<typename coordType>
void OutlineSquare(Point<coordType>& Point_a, Point<coordType>& Point_b, u32& Color, u32& Color_shade){
  const auto xa = Point_a.getX(), ya = Point_a.getY(), xb = Point_b.getX(), yb = Point_b.getY();
  for(auto x=xa;x <= xb;x=x+0.001){
    Point<coordType> pb {x, yb};
    Point<coordType> pt {x, ya};
    pb.set_ndc(org_x,org_y,cell_size);
    pt.set_ndc(org_x,org_y,cell_size);
    FillDot(pb, 1, Color, Color_shade);
    FillDot(pt, 1, Color, Color_shade);
    //DrawPixel(x, yb, Color); //Horiz bottom
    //DrawPixel(x, ya, Color); //Horiz top
  }//Points should make parallel lines
  for(auto y=ya;y <= yb;y=y+0.001){
    Point<coordType> pl {xb, y};
    Point<coordType> pr {xa, y};
    pl.set_ndc(org_x,org_y,cell_size);
    pr.set_ndc(org_x,org_y,cell_size);
    FillDot(pl, 1, Color, Color_shade);
    FillDot(pr, 1, Color, Color_shade);
    //DrawPixel(xb, y, Color); //Vert left
    //DrawPixel(xa, y, Color); //Vert right
  }
}

template<typename coordType>
void OutlineCircle(coordType& rad, u32& Color, u32& Color_shade){
  for(coordType t=-90;t < 90;t=t+0.001){
    auto x = ( rad * (1 - t*t) ) / ( t*t + 1 );
    auto y = ( rad * (2*t) )     / ( t*t + 1 );
    Point<coordType> pl {-x, y};
    Point<coordType> pr { x, y};
    pl.set_ndc(org_x,org_y,cell_size);
    pr.set_ndc(org_x,org_y,cell_size);
    FillDot(pl, 1, Color, Color_shade);
    FillDot(pr, 1, Color, Color_shade);
  }
}

template<typename coordType>
void FillSquare(Point<coordType>& Point_a, Point<coordType>& Point_b, u32& Color){
  const auto xa = Point_a.getX(), ya = Point_a.getY(), xb = Point_b.getX(), yb = Point_b.getY();
  for(auto x = xa;x <= xb;++x){
    for(auto y = ya;y <= yb;++y){
      DrawPixel(x, y, Color); //Right Horiz
    }
  }
}

template<typename coordType>
void FillDot(Point<coordType>& Point, unsigned&& px_size, u32& Color, u32& Color_shade){
  const coordType xa = Point.getX()-px_size, ya = Point.getY()-px_size, xb = Point.getX()+px_size, yb = Point.getY()+px_size;
  for(auto x=xa;x <= xb;++x){
    for(auto y=ya;y <= yb;++y){
      if(y < ya+static_cast<coordType>((yb-ya)/3) && x < xa+static_cast<coordType>((xb-xa)/3)){
        DrawPixel(x, y, Color_shade);// top left
      }
      else if(y < ya+static_cast<coordType>((yb-ya)/3) && x > xb-static_cast<coordType>((xb-xa)/3)){
        DrawPixel(x, y, Color_shade);// top right
      }
      else if(y > yb-static_cast<coordType>((yb-ya)/3) && x < xa+static_cast<coordType>((xb-xa)/3)){
        DrawPixel(x, y, Color_shade);// bottom left
      }
      else if(y > yb-static_cast<coordType>((yb-ya)/3) && x > xb-static_cast<coordType>((xb-xa)/3)){
        DrawPixel(x, y, Color_shade);// bottom right
      }
      else{
        DrawPixel(x, y, Color);
      }
    }
  }
}

template<typename coordType>
void HLine_expand(coordType& xa, coordType& ya, coordType& length, u32& Color) {
  for(auto x=1;x<=length/2;++x){
    DrawPixel(xa+x, ya, Color);
    DrawPixel(xa-x, ya, Color);
  }
}

template<typename coordType>
void VLine_expand(coordType xa, coordType ya, coordType& length, u32& Color){
  for(auto y=1;y<length/2;++y){
    DrawPixel(xa, ya+y, Color);
    DrawPixel(xa, ya-y, Color);
  }
}

template<typename coordType>
void DiagonalLine(Point<coordType>& Point_a, Point<coordType>& Point_b,
                  u32& Color, const std::string& dir){
  const auto xa = Point_a.getX(), ya = Point_a.getY(), xb = Point_b.getX(), yb = Point_b.getY();
  if(dir == "back" || dir == "downward"){
    /* /  */
    for(auto dx=xa,dy=yb;dx<=xb && dy>=ya;++dx,--dy){
      DrawPixel(dx, dy, Color);
    }
  }
  if(dir == "front" || dir == "forward" || dir == "upward"){
    /* \ */
     for(auto dx=xa,dy=ya;dx<=xb && dy<=yb;++dx,++dy){
      DrawPixel(dx, dy, Color);
    }
  }
}

//xa,ya and xb,yb are the top point and bottom point, that are adjacent to the right angle
template<typename coordType>
void OutlineRightTriangle(Point<coordType>& Point_a, Point<coordType>& Point_b,
                          u32& Color, bool vflip=false, bool hflip=false){
  auto xa = Point_a.getX(), ya = Point_a.getY(), xb = Point_b.getX(), yb = Point_b.getY();
  if((vflip == true && hflip == false) || (vflip == false && hflip == true)) {
    DiagonalLine<coordType>(Point_a, Point_b, Color, "back");/* /| */
  }
  else{
    DiagonalLine<coordType>(Point_a, Point_b, Color, "front");/* |\ */
  }
 for(auto dx=xa, dy=ya;dx <= xb && dy <= yb;++dy,++dx){
    if(vflip == false){
      if(yb < ya){
        yb = yb-ya;
      }
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
void OutlineEquilTriangle(Point<coordType>& Point_a, Point<coordType>& Point_b,
                          u32& Color, bool vflip=false){
  const auto xa = Point_a.getX(), ya = Point_a.getY(), xb = Point_b.getX(), yb = Point_b.getY();
  DiagonalLine<coordType>(Point_a, Point_b, Color, "back");
  Point<coordType> Point_c {xa+(xb-xa), ya};
  Point<coordType> Point_d {xb+(xb-xa), yb};
  DiagonalLine<coordType>(Point_c, Point_d, Color, "front");
  for(auto dx=xb-(xb-xa);dx <= xb+(yb-ya);++dx){
    DrawPixel(dx, yb, Color);//Horiz bot
  }
}

template<typename coordType>
void OutlineParallelogram(Point<coordType>& Point_a, Point<coordType>& Point_b,
                          u32& Color){
  const auto xa = Point_a.getX(), ya = Point_a.getY(), xb = Point_b.getX(), yb = Point_b.getY();
  Point<coordType> Point_c {xb,ya};
  Point<coordType> Point_d {xa+xb,yb};
    DiagonalLine<coordType>(Point_a, Point_b, Color, "front");
    DiagonalLine<coordType>(Point_c, Point_d, Color, "front");
    DiagonalLine<coordType>(Point_a, Point_b, Color, "back");
    DiagonalLine<coordType>(Point_c, Point_d, Color, "back");
    for(auto dx=xa, dx_=xb;dx <= xb;++dx,++dx_){
      DrawPixel(dx, ya, Color); //Horiz top
      DrawPixel(dx_, yb, Color);//Horiz bot
    }
    for(auto dx=xa, dx_=xb;dx <= xb;++dx,++dx_){
      DrawPixel(dx, yb, Color); //Horiz top
      DrawPixel(dx_, ya, Color);//Horiz bot
    }
}

void ClearScreen(u32 Color) {
    u32 *Pixel = (u32 *)BitmapMemory;
    for(int Index = 0;
        Index < BitmapWidth * BitmapHeight;
        ++Index) { *Pixel++ = Color; }
}

template<typename coordType>
void OutlineGrid(Point<coordType>& Point_a, Point<coordType>& Point_b,  coordType& cell_size,coordType& col_row_cell_n, u32& Color){
  coordType xa = Point_a.getX(), ya = Point_a.getY(), xb = Point_b.getX(), yb = Point_b.getY();
  for(coordType x = xa;x < xb;++x){
    for(coordType dy=ya,alt=0;dy < yb;dy=dy+(cell_size*cell_unit_interval),++alt){
      if(static_cast<int>(alt) % static_cast<int>(col_row_cell_n) == 0){
        DrawPixel(x, dy-1, Color);
        DrawPixel(x, dy, Color);
        DrawPixel(x, dy+1, Color);
      }
      else{
        DrawPixel(x, dy, Color);
      }
    }
  }
  for(coordType y = ya;y < yb;++y){
    for(coordType dx=xa,alt=0;dx < xb;dx=dx+(cell_size*cell_unit_interval),++alt){
      if(static_cast<int>(alt) % static_cast<int>(col_row_cell_n) == 0){
        DrawPixel(dx-1, y, Color);
        DrawPixel(dx,   y, Color);
        DrawPixel(dx+1, y, Color);
      }
      else{
        DrawPixel(dx, y, Color);
      }
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
    ClientWidth = (ClientRect.right*2) + (ClientRect.left*2); //Double the width
    ClientHeight = (ClientRect.bottom*2) + (ClientRect.top*2);//Double the height

    BitmapWidth = ClientWidth;
    BitmapHeight = ClientHeight;

    // Allocate memory for the bitmap
    int BytesPerPixel = 4;

    BitmapMemory = VirtualAlloc(0,BitmapWidth * BitmapHeight * BytesPerPixel,MEM_RESERVE|MEM_COMMIT,PAGE_READWRITE);

    // BitmapInfo struct for StretchDIBits
    BITMAPINFO BitmapInfo;
    BitmapInfo.bmiHeader.biSize = sizeof(BitmapInfo.bmiHeader);
    BitmapInfo.bmiHeader.biWidth = BitmapWidth;
    // Negative BitmapHeight makes top left as the coordinate system origin(0,0), otherwise its bottom left
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

        //Setup the grid/background/point of origin
        ClearScreen(0xFFFFFF);
        OutlineGrid(grid_top, grid_bot, cell_size, col_row_cell_n, grey_lite);

        auto x_axis_line_length = org_x*2;
        auto y_axis_line_length = org_y*2;

        HLine_expand(org_x, org_y, x_axis_line_length, green);//x-axis line
        VLine_expand(org_x, org_y, y_axis_line_length, green);//y-axis line

        //TESTING PURPOSE
        //-------------------------------
        // Assert the ndc's origin Point is set correctly, relative to ClientWindows width/height
        // PointInt test {};//default is x=0,y=0
        // test.set_ndc(org_x, org_y, cell_size);
        // assert(test.getX() == org_x);
        // assert(test.getY() == org_y);
        //-------------------------------

        //Sample of functions
        //-------------------------------
        //top = xa,ya and bot = ya,yb // these coordinates are top left and bottom right
        //Instead of using Device Coordinates:
        // PointInt top_dc = {(BitmapWidth/4)-15, (BitmapHeight/6)+80};
        // PointInt bot_dc = {(BitmapWidth/2)-30, (top_dc.getX() + top_dc.getY())};
        //Use cartesian coordinates, `normalized` DC:
        PointFloat top { -5, -5 };
        PointFloat bot {  5,  5 };
        float radius = 5.0;
        OutlineCircle(radius, purp_lite, purp_shade);
        // top.set_ndc(org_x, org_y, cell_size);
        // bot.set_ndc(org_x, org_y, cell_size);
        PointFloat center { 0, 0 };
        center.set_ndc(org_x, org_y, cell_size);
        FillDot(center, 1, purp_lite, purp_shade);                 /*        ->   "*"   */
        OutlineSquare     (top, bot, purp_lite, purp_shade);       /*        ->   "|_|" */
        // FillSquare        (top, bot, green_lite);               /*        ->   "|#|" */
        // VertexPointSquare (top, bot, purp_lite);                /*        ->   ": :" */
        // DiagonalLine      (top, bot, purp_lite, "front");       /*        ->    "\"  */
        // DiagonalLine      (top, bot, purp_lite, "back");        /*        ->    "/"  */
        // OutlineRightTriangle(top, bot, purp_lite, false, false);/*default ->   "|\"  */
        // OutlineRightTriangle(top, bot, purp_lite, true, false); /*        ->   "|/"  */
        // OutlineRightTriangle(top, bot, purp_lite, false, true); /*        ->   "/|"  */
        // OutlineRightTriangle(top, bot, purp_lite, true, true);  /*        ->   "\|"  */
        // OutlineParallelogram(top, bot, purp_lite);      /*        ->   "\\"  */
        // OutlineEquilTriangle(top, bot, purp_lite, false);       /*        ->   "/\"  */


        //-------------------------------

        StretchDIBits(DeviceContext,0, 0,BitmapWidth, BitmapHeight,0, 0,ClientWidth, ClientHeight,
                      BitmapMemory, &BitmapInfo,DIB_RGB_COLORS, SRCCOPY);
    }
    return 0;
}
