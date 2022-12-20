#define WIN32_LEAN_AND_MEAN
#define UNICODE
#include <windows.h>
// #include <cstdlib>
// #include <wingdi.h>//Don't include this directly, since windows.h implicitly includes that header
// #include <stdint.h>
#include <string>
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

// Draws a pixel at X, Y (from top left corner)
void DrawPixel(int X, int Y, u32& Color) {
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
void OutlineSquare(Point<coordType>& Point_a, Point<coordType>& Point_b, u32& Color){
  auto xa = Point_a.getX(), ya = Point_a.getY(), xb = Point_b.getX(), yb = Point_b.getY();
  for(auto x = xa;x < xb;++x){
    DrawPixel(x, yb, Color); //Horiz bottom
    DrawPixel(x, ya, Color); //Horiz top
  }//Points should make parallel lines
  for(auto y = ya;y < yb;++y){
    DrawPixel(xb, y, Color); //Vert left
    DrawPixel(xa, y, Color); //Vert right
  }
}

//Draws pixel points on each corner point(vertex)
template<typename coordType>
void VertexPointSquare(Point<coordType>& Point_a, Point<coordType>& Point_b, u32& Color){
  auto xa = Point_a.getX(), ya = Point_a.getY(), xb = Point_b.getX(), yb = Point_b.getY();
  DrawPixel(xb - xa, ya, Color);      //top left point
  DrawPixel(xb, ya, Color);           //top right point
  DrawPixel(xb - xa, yb, Color);      //bot left point
  DrawPixel(xb, yb, Color);           //bot right point
}

//Fills the area inside the coordinates
template<typename coordType>
void FillSquare(Point<coordType>& Point_a, Point<coordType>& Point_b, u32& Color, std::string&& dir="right"){
  auto xa = Point_a.getX(), ya = Point_a.getY(), xb = Point_b.getX(), yb = Point_b.getY();
  for(auto x = xa;x <= xb;++x){
    for(auto y = ya;y <= yb;++y){
      DrawPixel(x, y, Color); //Right Horiz
    }
  }
}

void HLine_expand(int& xa, int& ya, int& radius, u32& Color) {
  for(auto x=1;x<radius/2;++x){
    DrawPixel(xa+x, ya, Color);
    DrawPixel(xa-x, ya, Color);
  }
}

void VLine_expand(int xa, int ya, int& radius, u32& Color){
  for(auto y=1;y<radius/2;++y){
    DrawPixel(xa, ya+y, Color);
    DrawPixel(xa, ya-y, Color);
  }
}

template<typename coordType>
void DiagonalLine(Point<coordType>& Point_a, Point<coordType>& Point_b,
                  u32& Color, const std::string& dir){
  auto xa = Point_a.getX(), ya = Point_a.getY(), xb = Point_b.getX(), yb = Point_b.getY();
  if(dir == "back" || dir == "downward"){
    /* /  */
    for(auto dx=xa,dy=yb;dx<xb && dy>ya;++dx,--dy){
      DrawPixel(dx, dy, Color);
    }
  }
  if(dir == "front" || dir == "forward" || dir == "upward"){
    /* \ */
     for(auto dx=xa,dy=ya;dx<xb && dy<yb;++dx,++dy){
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
 for(auto dx=xa, dy=ya;dx < xb && dy < yb;++dy,++dx){
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
  auto xa = Point_a.getX(), ya = Point_a.getY(), xb = Point_b.getX(), yb = Point_b.getY();
  DiagonalLine<coordType>(Point_a, Point_b, Color, "back");
  Point<coordType> Point_c {xa+(xb-xa), ya};
  Point<coordType> Point_d {xb+(xb-xa), yb};
  DiagonalLine<coordType>(Point_c, Point_d, Color, "front");
  for(auto dx=xb-(xb-xa);dx < xb+(yb-ya);++dx){
    DrawPixel(dx, yb, Color);//Horiz bot
  }
}

template<typename coordType>
void OutlineParallelogram(Point<coordType>& Point_a, Point<coordType>& Point_b,
                          u32& Color){
  auto xa = Point_a.getX(), ya = Point_a.getY(), xb = Point_b.getX(), yb = Point_b.getY();
  Point<coordType> Point_c {xb,ya};
  Point<coordType> Point_d {xa+xb,yb};
    DiagonalLine<coordType>(Point_a, Point_b, Color, "front");
    DiagonalLine<coordType>(Point_c, Point_d, Color, "front");
    DiagonalLine<coordType>(Point_a, Point_b, Color, "back");
    DiagonalLine<coordType>(Point_c, Point_d, Color, "back");
    for(auto dx=xa, dx_=xb;dx < xb;++dx,++dx_){
      DrawPixel(dx, ya, Color); //Horiz top
      DrawPixel(dx_, yb, Color);//Horiz bot
    }
    for(auto dx=xa, dx_=xb;dx < xb;++dx,++dx_){
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
void OutlineGrid(Point<coordType>& Point_a, Point<coordType>& Point_b, int& cell_size,int& col_row_cell_n, u32& Color){
  auto xa = Point_a.getX(), ya = Point_a.getY(), xb = Point_b.getX(), yb = Point_b.getY();
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

    //Grid Setup
    int cell_size = 10;//sets pixel count of each cell(grid unit)
    int col_row_cell_n = 5;//sets the number of cells inside each col/row
    //padding offset for grid rectangle corners(in pixels)
    //offsets are needed or else bitmap program crashes
    auto grid_pad_left = 1, grid_pad_top = 1;
    using PointInt = Point<int>;//using integers for coordinates
    PointInt grid_top{grid_pad_left, grid_pad_top};
    PointInt grid_bot{1920,1080};//make a 1080 resolution grid

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

        //For accurate cartesian coordinate view, the axis origin points can be offset for even columns
        auto origin_axis = [&](int&& axis_len){
          auto ax_len_offset = axis_len % (cell_size*col_row_cell_n);
          auto cols_ = static_cast<int>((axis_len-ax_len_offset)/(cell_size*col_row_cell_n));
          auto fixed_origin = (cell_size*col_row_cell_n)*static_cast<int>(cols_/2);
          return fixed_origin;
        };

        //Drawing the axis lines without an offset
        //Maybe we can just `clamp` the width/height of each axis line
        auto org_x = origin_axis(grid_bot.getX()-grid_top.getX());
        auto org_y = origin_axis(grid_bot.getY()-grid_top.getY());
        auto x_axis_line_length = (org_x-(cell_size*col_row_cell_n))*2;
        auto y_axis_line_length = (org_y-(cell_size*col_row_cell_n))*2;
        HLine_expand(org_x, org_y, x_axis_line_length, green);//x-axis line
        VLine_expand(org_x, org_y, y_axis_line_length, green);//y-axis line

        //TESTING PURPOSE
        //-------------------------------
        // Assert the ndc's origin Point is set correctly, relative to ClientWindows width/height
        PointInt test = {0, 0};
        test.set_ndc(org_x, org_y, cell_size);
        assert(test.getX() == org_x);
        assert(test.getY() == org_y);
        //-------------------------------

        //Sample of functions
        //-------------------------------
        //top = xa,ya and bot = ya,yb // these coordinates are top left and bottom right
        //Instead of using Device Coordinates:
        //PointInt top_dc = {(BitmapWidth/4)-15, (BitmapHeight/6)+80};
        //PointInt bot_dc = {(BitmapWidth/2)-30, (top_dc.getX() + top_dc.getY())};
        //Use cartesian coordinates, `normalized` DC:
        PointInt top { -10, -10 };
        PointInt bot {  10,  10 };
        top.set_ndc(org_x, org_y, cell_size);
        bot.set_ndc(org_x, org_y, cell_size);
        // FillSquare(top, bot, bluegrey);        /*        ->   "|#|" */
        // OutlineSquare     (top, bot, purp_lite);                /*        ->   "|_|" */
        // VertexPointSquare (top, bot, purp_lite);                /*        ->   ": :" */
        // DiagonalLine      (top, bot, purp_lite, "front");       /*        ->    "\"  */
        // DiagonalLine      (top, bot, purp_lite, "back");        /*        ->    "/"  */
        // OutlineRightTriangle(top, bot, purp_lite, false, false);/*default ->   "|\"  */
        // OutlineRightTriangle(top, bot, purp_lite, true, false); /*        ->   "|/"  */
        // OutlineRightTriangle(top, bot, purp_lite, false, true); /*        ->   "/|"  */
        // OutlineRightTriangle(top, bot, purp_lite, true, true);  /*        ->   "\|"  */
        // OutlineParallelogram(top, bot, purp_lite);      /*        ->   "\\"  */
        OutlineEquilTriangle(top, bot, purp_lite, false);       /*        ->   "/\"  */
        //-------------------------------
        
        StretchDIBits(DeviceContext,0, 0,BitmapWidth, BitmapHeight,0, 0,ClientWidth, ClientHeight,
                      BitmapMemory, &BitmapInfo,DIB_RGB_COLORS, SRCCOPY);
    }
    return 0;
}
