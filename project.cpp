#if defined(UNICODE) && !defined(_UNICODE)
    #define _UNICODE
#elif defined(_UNICODE) && !defined(UNICODE)
    #define UNICODE
#endif

#include <tchar.h>
#include <windows.h>
#include <iostream>
#include <vector>
#include <fstream>
using namespace std;

// Menu IDs
#define IDM_WHITE_BG      101
#define IDM_CHANGE_CURSOR  102
#define IDM_CLEAR         103
#define IDM_SAVE          104
#define IDM_LOAD          105
#define IDM_EXIT          106

// Shape and Algorithm IDs
#define IDM_LINE_DDA          201
#define IDM_LINE_MIDPOINT     202
#define IDM_LINE_PARAMETRIC   203

#define IDM_CIRCLE_DIRECT     211
#define IDM_CIRCLE_POLAR      212
#define IDM_CIRCLE_ITER_POLAR 213
#define IDM_CIRCLE_MIDPOINT   214
#define IDM_CIRCLE_MOD_MID    215

#define IDM_ELLIPSE_DIRECT    221
#define IDM_ELLIPSE_POLAR     222
#define IDM_ELLIPSE_MIDPOINT  223

#define IDM_FILL_CIRCLE_LINES     231
#define IDM_FILL_CIRCLE_CIRCLES   232
#define IDM_FILL_SQUARE_HERMIT    233
#define IDM_FILL_RECTANGLE_BEZIER 234
#define IDM_FILL_CONVEX           235
#define IDM_FILL_NONCONVEX        236
#define IDM_FLOOD_FILL_REC        237
#define IDM_FLOOD_FILL_NONREC     238

#define IDM_CLIP_POINT_RECT   241
#define IDM_CLIP_LINE_RECT    242
#define IDM_CLIP_POLYGON_RECT 243
#define IDM_CLIP_POINT_SQUARE 244
#define IDM_CLIP_LINE_SQUARE  245

#define IDM_CARDINAL_SPLINE   251

// Color selection IDs
#define IDM_COLOR_RED     301
#define IDM_COLOR_GREEN   302
#define IDM_COLOR_BLUE    303
#define IDM_COLOR_BLACK   304
#define IDM_COLOR_CUSTOM  305

// Global variables
COLORREF currentColor = RGB(0, 0, 0); // Default to black
vector<POINT> points; // Stores points for drawing
bool isDrawing = false;
int currentAlgorithm = 0;
HCURSOR customCursor = NULL;

// Forward declarations for drawing functions
void DrawDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color);
void DrawMidpointLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color);
void DrawParametricLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color);

void DrawDirectCircle(HDC hdc, int xc, int yc, int radius, COLORREF color);
void DrawPolarCircle(HDC hdc, int xc, int yc, int radius, COLORREF color);
void DrawIterativePolarCircle(HDC hdc, int xc, int yc, int radius, COLORREF color);
void DrawMidpointCircle(HDC hdc, int xc, int yc, int radius, COLORREF color);
void DrawModifiedMidpointCircle(HDC hdc, int xc, int yc, int radius, COLORREF color);

void DrawDirectEllipse(HDC hdc, int xc, int yc, int a, int b, COLORREF color);
void DrawPolarEllipse(HDC hdc, int xc, int yc, int a, int b, COLORREF color);
void DrawMidpointEllipse(HDC hdc, int xc, int yc, int a, int b, COLORREF color);

void FillCircleWithLines(HDC hdc, int xc, int yc, int radius, int quarter, COLORREF color);
void FillCircleWithCircles(HDC hdc, int xc, int yc, int radius, int quarter, COLORREF color);
void FillSquareWithHermit(HDC hdc, int left, int top, int size, COLORREF color);
void FillRectangleWithBezier(HDC hdc, int left, int top, int width, int height, COLORREF color);
void ConvexFill(HDC hdc, const POINT* vertices, int count, COLORREF color);
void NonConvexFill(HDC hdc, const POINT* vertices, int count, COLORREF color);
void FloodFillRecursive(HDC hdc, int x, int y, COLORREF fillColor, COLORREF borderColor);
void FloodFillNonRecursive(HDC hdc, int x, int y, COLORREF fillColor, COLORREF borderColor);

void DrawCardinalSpline(HDC hdc, const POINT* points, int n, double tension, COLORREF color);

void PointClippingRectangle(HDC hdc, int x, int y, int left, int top, int right, int bottom, COLORREF color);
void LineClippingRectangle(HDC hdc, int x1, int y1, int x2, int y2, int left, int top, int right, int bottom, COLORREF color);
void PolygonClippingRectangle(HDC hdc, const POINT* vertices, int count, int left, int top, int right, int bottom, COLORREF color);
void PointClippingSquare(HDC hdc, int x, int y, int left, int top, int size, COLORREF color);
void LineClippingSquare(HDC hdc, int x1, int y1, int x2, int y2, int left, int top, int size, COLORREF color);

void SaveDrawing(HWND hwnd);
void LoadDrawing(HWND hwnd);

LRESULT CALLBACK WindowProcedure(HWND, UINT, WPARAM, LPARAM);
void AddMenus(HWND hwnd);

TCHAR szClassName[] = _T("2DDrawingApp");

int WINAPI WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance,
                   LPSTR lpszArgument, int nCmdShow) {
    HWND hwnd;
    MSG messages;
    WNDCLASSEX wincl;

    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szClassName;
    wincl.lpfnWndProc = WindowProcedure;
    wincl.style = CS_DBLCLKS;
    wincl.cbSize = sizeof(WNDCLASSEX);
    wincl.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hIconSm = LoadIcon(NULL, IDI_APPLICATION);
    wincl.hCursor = LoadCursor(NULL, IDC_ARROW);
    wincl.lpszMenuName = NULL;
    wincl.cbClsExtra = 0;
    wincl.cbWndExtra = 0;
    wincl.hbrBackground = (HBRUSH)(COLOR_WINDOW+1); // Default to white background

    if (!RegisterClassEx(&wincl))
        return 0;

    hwnd = CreateWindowEx(
        0, szClassName, _T("2D Drawing Program"),
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        HWND_DESKTOP, NULL, hThisInstance, NULL
    );

    AddMenus(hwnd);

    ShowWindow(hwnd, nCmdShow);

    while (GetMessage(&messages, NULL, 0, 0)) {
        TranslateMessage(&messages);
        DispatchMessage(&messages);
    }

    return messages.wParam;
}

void AddMenus(HWND hwnd) {
    HMENU hMenu = CreateMenu();
    HMENU hFileMenu = CreatePopupMenu();
    HMENU hDrawMenu = CreatePopupMenu();
    HMENU hLineMenu = CreatePopupMenu();
    HMENU hCircleMenu = CreatePopupMenu();
    HMENU hEllipseMenu = CreatePopupMenu();
    HMENU hFillMenu = CreatePopupMenu();
    HMENU hClipMenu = CreatePopupMenu();
    HMENU hColorMenu = CreatePopupMenu();
    HMENU hSettingsMenu = CreatePopupMenu();

    // File menu
    AppendMenu(hFileMenu, MF_STRING, IDM_SAVE, "Save");
    AppendMenu(hFileMenu, MF_STRING, IDM_LOAD, "Load");
    AppendMenu(hFileMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFileMenu, MF_STRING, IDM_EXIT, "Exit");

    // Line algorithms
    AppendMenu(hLineMenu, MF_STRING, IDM_LINE_DDA, "DDA");
    AppendMenu(hLineMenu, MF_STRING, IDM_LINE_MIDPOINT, "Midpoint");
    AppendMenu(hLineMenu, MF_STRING, IDM_LINE_PARAMETRIC, "Parametric");

    // Circle algorithms
    AppendMenu(hCircleMenu, MF_STRING, IDM_CIRCLE_DIRECT, "Direct");
    AppendMenu(hCircleMenu, MF_STRING, IDM_CIRCLE_POLAR, "Polar");
    AppendMenu(hCircleMenu, MF_STRING, IDM_CIRCLE_ITER_POLAR, "Iterative Polar");
    AppendMenu(hCircleMenu, MF_STRING, IDM_CIRCLE_MIDPOINT, "Midpoint");
    AppendMenu(hCircleMenu, MF_STRING, IDM_CIRCLE_MOD_MID, "Modified Midpoint");

    // Ellipse algorithms
    AppendMenu(hEllipseMenu, MF_STRING, IDM_ELLIPSE_DIRECT, "Direct");
    AppendMenu(hEllipseMenu, MF_STRING, IDM_ELLIPSE_POLAR, "Polar");
    AppendMenu(hEllipseMenu, MF_STRING, IDM_ELLIPSE_MIDPOINT, "Midpoint");

    // Fill algorithms
    AppendMenu(hFillMenu, MF_STRING, IDM_FILL_CIRCLE_LINES, "Circle with Lines");
    AppendMenu(hFillMenu, MF_STRING, IDM_FILL_CIRCLE_CIRCLES, "Circle with Circles");
    AppendMenu(hFillMenu, MF_STRING, IDM_FILL_SQUARE_HERMIT, "Square with Hermit");
    AppendMenu(hFillMenu, MF_STRING, IDM_FILL_RECTANGLE_BEZIER, "Rectangle with Bezier");
    AppendMenu(hFillMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFillMenu, MF_STRING, IDM_FILL_CONVEX, "Convex Fill");
    AppendMenu(hFillMenu, MF_STRING, IDM_FILL_NONCONVEX, "Non-Convex Fill");
    AppendMenu(hFillMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hFillMenu, MF_STRING, IDM_FLOOD_FILL_REC, "Flood Fill (Recursive)");
    AppendMenu(hFillMenu, MF_STRING, IDM_FLOOD_FILL_NONREC, "Flood Fill (Non-Recursive)");

    // Clipping algorithms
    AppendMenu(hClipMenu, MF_STRING, IDM_CLIP_POINT_RECT, "Point (Rectangle)");
    AppendMenu(hClipMenu, MF_STRING, IDM_CLIP_LINE_RECT, "Line (Rectangle)");
    AppendMenu(hClipMenu, MF_STRING, IDM_CLIP_POLYGON_RECT, "Polygon (Rectangle)");
    AppendMenu(hClipMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hClipMenu, MF_STRING, IDM_CLIP_POINT_SQUARE, "Point (Square)");
    AppendMenu(hClipMenu, MF_STRING, IDM_CLIP_LINE_SQUARE, "Line (Square)");

    // Color menu
    AppendMenu(hColorMenu, MF_STRING, IDM_COLOR_RED, "Red");
    AppendMenu(hColorMenu, MF_STRING, IDM_COLOR_GREEN, "Green");
    AppendMenu(hColorMenu, MF_STRING, IDM_COLOR_BLUE, "Blue");
    AppendMenu(hColorMenu, MF_STRING, IDM_COLOR_BLACK, "Black");
    AppendMenu(hColorMenu, MF_STRING, IDM_COLOR_CUSTOM, "Custom...");

    // Settings menu
    AppendMenu(hSettingsMenu, MF_STRING, IDM_WHITE_BG, "White Background");
    AppendMenu(hSettingsMenu, MF_STRING, IDM_CHANGE_CURSOR, "Change Cursor");

    // Draw menu (main drawing options)
    AppendMenu(hDrawMenu, MF_POPUP, (UINT_PTR)hLineMenu, "Line");
    AppendMenu(hDrawMenu, MF_POPUP, (UINT_PTR)hCircleMenu, "Circle");
    AppendMenu(hDrawMenu, MF_POPUP, (UINT_PTR)hEllipseMenu, "Ellipse");
    AppendMenu(hDrawMenu, MF_STRING, IDM_CARDINAL_SPLINE, "Cardinal Spline");
    AppendMenu(hDrawMenu, MF_POPUP, (UINT_PTR)hFillMenu, "Fill");
    AppendMenu(hDrawMenu, MF_POPUP, (UINT_PTR)hClipMenu, "Clipping");
    AppendMenu(hDrawMenu, MF_SEPARATOR, 0, NULL);
    AppendMenu(hDrawMenu, MF_STRING, IDM_CLEAR, "Clear Screen");

    // Main menu bar
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hFileMenu, "File");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hDrawMenu, "Draw");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hColorMenu, "Color");
    AppendMenu(hMenu, MF_POPUP, (UINT_PTR)hSettingsMenu, "Settings");

    SetMenu(hwnd, hMenu);
}

LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
    HDC hdc;
    PAINTSTRUCT ps;
    static POINT startPoint, endPoint;

    switch (message) {
        case WM_COMMAND:
            switch (LOWORD(wParam)) {
                // File menu
                case IDM_SAVE:
                    SaveDrawing(hwnd);
                    break;
                case IDM_LOAD:
                    LoadDrawing(hwnd);
                    break;
                case IDM_EXIT:
                    PostQuitMessage(0);
                    break;

                // Settings menu
                case IDM_WHITE_BG:
                    SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG_PTR)GetStockObject(WHITE_BRUSH));
                    InvalidateRect(hwnd, NULL, TRUE);
                    break;
                case IDM_CHANGE_CURSOR:
                    // Implement cursor change logic here
                    // Example: customCursor = LoadCursorFromFile(...);
                    // SetCursor(customCursor);
                    break;

                // Color menu
                case IDM_COLOR_RED:
                    currentColor = RGB(255, 0, 0);
                    break;
                case IDM_COLOR_GREEN:
                    currentColor = RGB(0, 255, 0);
                    break;
                case IDM_COLOR_BLUE:
                    currentColor = RGB(0, 0, 255);
                    break;
                case IDM_COLOR_BLACK:
                    currentColor = RGB(0, 0, 0);
                    break;
                case IDM_COLOR_CUSTOM:
                    // Implement color picker dialog here
                    break;

                // Clear screen
                case IDM_CLEAR:
                    InvalidateRect(hwnd, NULL, TRUE);
                    points.clear();
                    break;

                // Line algorithms
                case IDM_LINE_DDA:
                    currentAlgorithm = IDM_LINE_DDA;
                    break;
                case IDM_LINE_MIDPOINT:
                    currentAlgorithm = IDM_LINE_MIDPOINT;
                    break;
                case IDM_LINE_PARAMETRIC:
                    currentAlgorithm = IDM_LINE_PARAMETRIC;
                    break;

                // Circle algorithms
                case IDM_CIRCLE_DIRECT:
                    currentAlgorithm = IDM_CIRCLE_DIRECT;
                    break;
                case IDM_CIRCLE_POLAR:
                    currentAlgorithm = IDM_CIRCLE_POLAR;
                    break;
                case IDM_CIRCLE_ITER_POLAR:
                    currentAlgorithm = IDM_CIRCLE_ITER_POLAR;
                    break;
                case IDM_CIRCLE_MIDPOINT:
                    currentAlgorithm = IDM_CIRCLE_MIDPOINT;
                    break;
                case IDM_CIRCLE_MOD_MID:
                    currentAlgorithm = IDM_CIRCLE_MOD_MID;
                    break;

                // Ellipse algorithms
                case IDM_ELLIPSE_DIRECT:
                    currentAlgorithm = IDM_ELLIPSE_DIRECT;
                    break;
                case IDM_ELLIPSE_POLAR:
                    currentAlgorithm = IDM_ELLIPSE_POLAR;
                    break;
                case IDM_ELLIPSE_MIDPOINT:
                    currentAlgorithm = IDM_ELLIPSE_MIDPOINT;
                    break;

                // Fill algorithms
                case IDM_FILL_CIRCLE_LINES:
                    currentAlgorithm = IDM_FILL_CIRCLE_LINES;
                    break;
                case IDM_FILL_CIRCLE_CIRCLES:
                    currentAlgorithm = IDM_FILL_CIRCLE_CIRCLES;
                    break;
                case IDM_FILL_SQUARE_HERMIT:
                    currentAlgorithm = IDM_FILL_SQUARE_HERMIT;
                    break;
                case IDM_FILL_RECTANGLE_BEZIER:
                    currentAlgorithm = IDM_FILL_RECTANGLE_BEZIER;
                    break;
                case IDM_FILL_CONVEX:
                    currentAlgorithm = IDM_FILL_CONVEX;
                    break;
                case IDM_FILL_NONCONVEX:
                    currentAlgorithm = IDM_FILL_NONCONVEX;
                    break;
                case IDM_FLOOD_FILL_REC:
                    currentAlgorithm = IDM_FLOOD_FILL_REC;
                    break;
                case IDM_FLOOD_FILL_NONREC:
                    currentAlgorithm = IDM_FLOOD_FILL_NONREC;
                    break;

                // Clipping algorithms
                case IDM_CLIP_POINT_RECT:
                    currentAlgorithm = IDM_CLIP_POINT_RECT;
                    break;
                case IDM_CLIP_LINE_RECT:
                    currentAlgorithm = IDM_CLIP_LINE_RECT;
                    break;
                case IDM_CLIP_POLYGON_RECT:
                    currentAlgorithm = IDM_CLIP_POLYGON_RECT;
                    break;
                case IDM_CLIP_POINT_SQUARE:
                    currentAlgorithm = IDM_CLIP_POINT_SQUARE;
                    break;
                case IDM_CLIP_LINE_SQUARE:
                    currentAlgorithm = IDM_CLIP_LINE_SQUARE;
                    break;

                // Cardinal Spline
                case IDM_CARDINAL_SPLINE:
                    currentAlgorithm = IDM_CARDINAL_SPLINE;
                    break;
            }
            break;

        case WM_LBUTTONDOWN:
            startPoint.x = LOWORD(lParam);
            startPoint.y = HIWORD(lParam);
            points.push_back(startPoint);
            isDrawing = true;
            break;

        case WM_LBUTTONUP:
            if (isDrawing) {
                endPoint.x = LOWORD(lParam);
                endPoint.y = HIWORD(lParam);
                points.push_back(endPoint);

                hdc = GetDC(hwnd);

                // Call appropriate drawing function based on currentAlgorithm
                switch (currentAlgorithm) {
                    case IDM_LINE_DDA:
                        DrawDDA(hdc, startPoint.x, startPoint.y, endPoint.x, endPoint.y, currentColor);
                        break;
                    case IDM_LINE_MIDPOINT:
                        DrawMidpointLine(hdc, startPoint.x, startPoint.y, endPoint.x, endPoint.y, currentColor);
                        break;
                    case IDM_LINE_PARAMETRIC:
                        DrawParametricLine(hdc, startPoint.x, startPoint.y, endPoint.x, endPoint.y, currentColor);
                        break;

                    // Add cases for other algorithms...

                    default:
                        // Default drawing (line)
                        MoveToEx(hdc, startPoint.x, startPoint.y, NULL);
                        LineTo(hdc, endPoint.x, endPoint.y);
                }

                ReleaseDC(hwnd, hdc);
                isDrawing = false;
            }
            break;

        case WM_MOUSEMOVE:
            if (isDrawing) {
                // Handle drawing while mouse is moving (for freehand drawing)
                // Optional: Implement if needed
            }
            break;

        case WM_PAINT:
            hdc = BeginPaint(hwnd, &ps);
            // Redraw all saved shapes here if needed
            EndPaint(hwnd, &ps);
            break;

        case WM_DESTROY:
            PostQuitMessage(0);
            break;

        default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0;
}

void SaveDrawing(HWND hwnd) {
    // Implement saving drawing data to file
    // This would typically save the points vector and other drawing information
    ofstream outFile("drawing.dat");
    if (outFile.is_open()) {
        for (const auto& point : points) {
            outFile << point.x << " " << point.y << "\n";
        }
        outFile.close();
        MessageBox(hwnd, "Drawing saved successfully!", "Save", MB_OK);
    } else {
        MessageBox(hwnd, "Error saving drawing!", "Error", MB_OK | MB_ICONERROR);
    }
}

void LoadDrawing(HWND hwnd) {
    // Implement loading drawing data from file
    ifstream inFile("drawing.dat");
    if (inFile.is_open()) {
        points.clear();
        POINT pt;
        while (inFile >> pt.x >> pt.y) {
            points.push_back(pt);
        }
        inFile.close();
        InvalidateRect(hwnd, NULL, TRUE); // Redraw window
        MessageBox(hwnd, "Drawing loaded successfully!", "Load", MB_OK);
    } else {
        MessageBox(hwnd, "Error loading drawing!", "Error", MB_OK | MB_ICONERROR);
    }
}

// Placeholder implementations for drawing algorithms
void DrawDDA(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {
    
}

void DrawMidpointLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color) {

}

void DrawParametricLine(HDC hdc, int x1, int y1, int x2, int y2, COLORREF color){

}

void DrawDirectCircle(HDC hdc, int xc, int yc, int radius, COLORREF color){

}
void DrawPolarCircle(HDC hdc, int xc, int yc, int radius, COLORREF color){

}
void DrawIterativePolarCircle(HDC hdc, int xc, int yc, int radius, COLORREF color){

}
void DrawMidpointCircle(HDC hdc, int xc, int yc, int radius, COLORREF color){

}
void DrawModifiedMidpointCircle(HDC hdc, int xc, int yc, int radius, COLORREF color){

}

void DrawDirectEllipse(HDC hdc, int xc, int yc, int a, int b, COLORREF color){

}
void DrawPolarEllipse(HDC hdc, int xc, int yc, int a, int b, COLORREF color){

}
void DrawMidpointEllipse(HDC hdc, int xc, int yc, int a, int b, COLORREF color){

}

void FillCircleWithLines(HDC hdc, int xc, int yc, int radius, int quarter, COLORREF color){

}
void FillCircleWithCircles(HDC hdc, int xc, int yc, int radius, int quarter, COLORREF color){

}
void FillSquareWithHermit(HDC hdc, int left, int top, int size, COLORREF color){

}
void FillRectangleWithBezier(HDC hdc, int left, int top, int width, int height, COLORREF color){

}
void ConvexFill(HDC hdc, const POINT* vertices, int count, COLORREF color){

}
void NonConvexFill(HDC hdc, const POINT* vertices, int count, COLORREF color){

}
void FloodFillRecursive(HDC hdc, int x, int y, COLORREF fillColor, COLORREF borderColor){

}
void FloodFillNonRecursive(HDC hdc, int x, int y, COLORREF fillColor, COLORREF borderColor){

}

void DrawCardinalSpline(HDC hdc, const POINT* points, int n, double tension, COLORREF color){

}

void PointClippingRectangle(HDC hdc, int x, int y, int left, int top, int right, int bottom, COLORREF color){

}
void LineClippingRectangle(HDC hdc, int x1, int y1, int x2, int y2, int left, int top, int right, int bottom, COLORREF color){

}
void PolygonClippingRectangle(HDC hdc, const POINT* vertices, int count, int left, int top, int right, int bottom, COLORREF color){

}
void PointClippingSquare(HDC hdc, int x, int y, int left, int top, int size, COLORREF color){

}
void LineClippingSquare(HDC hdc, int x1, int y1, int x2, int y2, int left, int top, int size, COLORREF color){

}

void SaveDrawing(HWND hwnd);
void LoadDrawing(HWND hwnd);
