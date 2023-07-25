/*
* Dependencies:
*  gdi32
*  (kernel32)
*  user32
*  (comctl32)
*
*/

#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <windows.h>
#include <stdint.h>
#include <windowsx.h>
#include <fstream>
#include <string>

#define KEY_SHIFTED     0x8000
#define KEY_TOGGLED     0x0001

struct COLOR {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
};

COLOR bgColor = {0, 0 , 255}; // цвет фона окна
COLOR cellColor = {255, 0, 0}; // цвет сетки
long matrixSize = 10; // N
long wWidth = 320; // ширина окна
long wHeight = 240; // высота окна
int** matrix;

enum cChanges {
    red_up,
    red_down,
    green_up,
    green_down,
    blue_up,
    blue_down
};

cChanges state = green_up;
const TCHAR szWinClass[] = _T("Win32SampleApp");
const TCHAR szWinName[] = _T("Win32SampleWindow");
HWND hwnd;               /* This is the handle for our window */
HBRUSH hBrush;           /* Current brush */



void StateSetter() {
    if (cellColor.r == 255) {
        if (cellColor.b == 0) {
            state = green_up;
        }
        if (cellColor.g == 0) {
            state = blue_down;
        }
    }
    if (cellColor.g == 255) {
        if (cellColor.r == 0) {
            state = blue_up;
        }
        if (cellColor.b == 0) {
            state = red_down;
        }
    }
    if (cellColor.b == 255) {
        if (cellColor.r == 0) {
            state = green_down;
        }
        if (cellColor.g == 0) {
            state = red_up;
        }
    }
}
/* Runs Notepad */
void RunNotepad(void)
{
    STARTUPINFO sInfo;

    PROCESS_INFORMATION pInfo;

    ZeroMemory(&sInfo, sizeof(STARTUPINFO));

    puts("Starting Notepad...");
    CreateProcess(_T("C:\\Windows\\Notepad.exe"),
        NULL, NULL, NULL, FALSE, 0, NULL, NULL, &sInfo, &pInfo);
}

/*  This function is called by the Windows function DispatchMessage()  */
LRESULT CALLBACK WindowProcedure(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    PAINTSTRUCT ps;
    HDC hdc;
    int x;
    int y;
    HBRUSH hTempBrush;

    switch (message)                  /* handle the messages */
    {
    case WM_LBUTTONUP:
    {
        x = GET_X_LPARAM(lParam);
        y = GET_Y_LPARAM(lParam);
        if (matrix[x / (wWidth / matrixSize)][y / (wHeight / matrixSize)] == 0) {
            matrix[x / (wWidth / matrixSize)][y / (wHeight / matrixSize)] = 1;
        }
        
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    case WM_RBUTTONUP:
    {
        x = GET_X_LPARAM(lParam);
        y = GET_Y_LPARAM(lParam);
        if (matrix[x / (wWidth / matrixSize)][y / (wHeight / matrixSize)] == 0) {
            matrix[x / (wWidth / matrixSize)][y / (wHeight / matrixSize)] = 2 ;
        }
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    case WM_MOUSEWHEEL:
    {
        int delta = GET_WHEEL_DELTA_WPARAM(wParam);
        if (delta > 0) {
            switch (state) {
            case(green_up):
                cellColor.g += 15;
                if (cellColor.b == 255) {
                    state = red_down;
                }
                break;
            case(red_down):
                cellColor.r -= 15;
                if (cellColor.r == 0) {
                    state = blue_up;
                }
                break;
            case(blue_up):
                cellColor.b += 15;
                if (cellColor.b == 255) {
                    state = green_down;
                }
                break;
            case(green_down):
                cellColor.g -= 15;
                if (cellColor.g == 0) {
                    state = red_up;
                }
                break;
            case(red_up):
                cellColor.r += 15;
                if (cellColor.r == 255) {
                    state = blue_down;
                }
                break;
            case(blue_down):
                cellColor.b -= 15;
                if (cellColor.b == 0) {
                    state = green_up;
                }
                break;
            }
        }
        else if (delta < 0) {
            switch (state) {
            case (green_up):
                cellColor.g -= 15;
                if (cellColor.g == 0) {
                    state = blue_down;
                }
                break;
            case (blue_down):
                cellColor.b += 15;
                if (cellColor.b == 255) {
                    state = red_up;
                }
                break;
            case (red_up):
                 cellColor.r -= 15;
                 if (cellColor.r == 0) {
                     state = green_down;
                 }
                 break;
            case (green_down):
                cellColor.g += 15;
                if (cellColor.b == 255) {
                    state = blue_up;
                }
                break;
            case (blue_up):
                cellColor.b -= 15;
                if (cellColor.r == 0) {
                    state = red_down;
                }
                break;
            case (red_down):
                cellColor.r += 15;
                if (cellColor.g == 255) {
                    state = green_up;
                }
                break;
            }
        }
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    case WM_KEYDOWN:
        switch (wParam)
        {
        case 67:
            if (GetKeyState(VK_SHIFT) & KEY_SHIFTED) {
                RunNotepad();
            }
            return 0;
        case 81:
            /*if (GetKeyState(VK_CONTROL) & KEY_SHIFTED) {
                DestroyWindow(hwnd);
            }*/
            PostQuitMessage(0);
            return 0;
        case VK_ESCAPE:
            // DestroyWindow(hwnd);
            PostQuitMessage(0);
            return 0;
        case VK_RETURN:
            bgColor = { (uint8_t)(rand() % 256), (uint8_t)(rand() % 256), (uint8_t)(rand() % 256) };
            hBrush = CreateSolidBrush(RGB(bgColor.r, bgColor.g, bgColor.b));
            hTempBrush = (HBRUSH)(DWORD_PTR)SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG)hBrush);
            DeleteObject(hTempBrush);
            InvalidateRect(hwnd, NULL, TRUE);
            return 0;
        }
    case WM_SIZE:
    {
        RECT rect;
        GetClientRect(hwnd, &rect);
        wWidth = rect.right;
        wHeight = rect.bottom;
        InvalidateRect(hwnd, NULL, TRUE);
        return 0;
    }
    case WM_CLOSE:
        /*RECT rect;
        GetClientRect(hwnd, &rect);
        wWidth = rect.right;
        wHeight = rect.bottom;*/
        PostQuitMessage(0);
        return 0;
    case WM_PAINT:
        hdc = BeginPaint(hwnd, &ps);
        RECT wSize;
        GetClientRect(hwnd, &wSize);
        FillRect(hdc, &wSize, hBrush);
        HPEN hPen = CreatePen(PS_SOLID, NULL, COLORREF(RGB(cellColor.r, cellColor.g, cellColor.b)));
        HPEN hDefaultPen = (HPEN)SelectObject(hdc, hPen);
        for (long i = 0; i < matrixSize; i++) {
            MoveToEx(hdc, i * wWidth / matrixSize, 0, NULL); // wSize.right = wWidth; wSize.bottpm = wHeight;
            LineTo(hdc, i * wWidth / matrixSize, wHeight);
            MoveToEx(hdc, 0, i * wHeight / matrixSize, NULL);
            LineTo(hdc, wWidth, i * wHeight / matrixSize);
        }
        hPen = (HPEN)SelectObject(hdc, hDefaultPen);
        DeleteObject(hPen);

        hPen = CreatePen(PS_SOLID, NULL, COLORREF(RGB(0, 0, 0)));
        hDefaultPen = (HPEN)SelectObject(hdc, hPen);

        HBRUSH hDefaultBrush = (HBRUSH)SelectObject(hdc, hBrush);
        for (int i = 0; i < matrixSize; i++) {
            for (int j = 0; j < matrixSize; j++) {
                if (matrix[i][j] == 1) {
                    Ellipse(hdc, i * wWidth / matrixSize, j * wHeight / matrixSize, (i + 1) * wWidth / matrixSize, (j + 1) * wHeight / matrixSize);
                }
                if (matrix[i][j] == 2) {
                    MoveToEx(hdc, i * wWidth / matrixSize, j * wHeight / matrixSize, NULL);
                    LineTo(hdc, (i + 1) * wWidth / matrixSize, (j + 1) * wHeight / matrixSize);
                    MoveToEx(hdc, i * wWidth / matrixSize, (j + 1) * wHeight / matrixSize, NULL);
                    LineTo(hdc, (i + 1) * wWidth / matrixSize, j * wHeight / matrixSize);
                }
            }
        }  
        

        DeleteObject(hPen);
        DeleteObject(hDefaultBrush);

        EndPaint(hwnd, &ps);
        return 0;
    }
    

    /* for messages that we don't deal with */
    return DefWindowProc(hwnd, message, wParam, lParam);
}

int main(int argc, char** argv)
{
    BOOL bMessageOk;
    MSG message;            /* Here message to the application are saved */
    WNDCLASS wincl = { 0 };         /* Data structure for the windowclass */

    /* Harcode show command num when use non-winapi entrypoint */
    int nCmdShow = SW_SHOW;
    /* Get handle */
    HINSTANCE hThisInstance = GetModuleHandle(NULL);

    /* The Window structure */
    wincl.hInstance = hThisInstance;
    wincl.lpszClassName = szWinClass;
    wincl.lpfnWndProc = WindowProcedure;      /* This function is called by Windows */

    /* Use custom brush to paint the background of the window */

    std::ifstream fin;
    fin.open("cfg.txt");
    if (fin) {
        fin >> matrixSize >> wWidth >> wHeight >> bgColor.r >> bgColor.g >> bgColor.b >> cellColor.r >> cellColor.g >> cellColor.b;
    }
    fin.close();

    StateSetter();

    hBrush = CreateSolidBrush(RGB(bgColor.r, bgColor.g, bgColor.b));
    wincl.hbrBackground = hBrush;
    

    /* Register the window class, and if it fails quit the program */
    if (!RegisterClass(&wincl))
        return 0;
    /* The class is registered, let's create the program*/
    hwnd = CreateWindow(
        szWinClass,          /* Classname */
        szWinName,       /* Title Text */
        WS_OVERLAPPEDWINDOW, /* default window */
        CW_USEDEFAULT,       /* Windows decides the position */
        CW_USEDEFAULT,       /* where the window ends up on the screen */
        wWidth,                 /* The programs width */
        wHeight,                 /* and height in pixels */
        HWND_DESKTOP,        /* The window is a child-window to desktop */
        NULL,                /* No menu */
        hThisInstance,       /* Program Instance handler */
        NULL                 /* No Window Creation data */
    );
    

    /* Make the window visible on the screen */

    if (argc > 1) {
        if (atoi(argv[1]) > 0) {
            matrixSize = atoi(argv[1]);
        }
    }
    matrix = new int* [matrixSize];
    for (int i = 0; i < matrixSize; i++) {
        matrix[i] = new int[matrixSize];
        for (int j = 0; j < matrixSize; j++) {
            matrix[i][j] = 0;
        }
    }

    ShowWindow(hwnd, nCmdShow);
    /* Run the message loop. It will run until GetMessage() returns 0 */
    while ((bMessageOk = GetMessage(&message, NULL, 0, 0)) != 0)
    {
        /* Yep, fuck logic: BOOL mb not only 1 or 0.
         * See msdn at https://msdn.microsoft.com/en-us/library/windows/desktop/ms644936(v=vs.85).aspx
         */
        if (bMessageOk == -1)
        {
            puts("Suddenly, GetMessage failed! You can call GetLastError() to see what happend");
            break;
        }
        /* Translate virtual-key message into character message */
        TranslateMessage(&message);
        /* Send message to WindowProcedure */
        DispatchMessage(&message);
    }

    /* Cleanup stuff */
    for (int i = 0; i < matrixSize; i++) {
        delete[] matrix[i];
    }
    delete[] matrix;

    std::ofstream fout;
    fout.open("cfg.txt", std::ofstream::out | std::ofstream::trunc);
    fout << matrixSize << "\n" << wWidth << "\n" << wHeight << "\n"
        << bgColor.r << "\n" << bgColor.g << "\n" << bgColor.b << "\n"
        << cellColor.r << "\n" << cellColor.g << "\n" << cellColor.b << "\n";
        
    fout.close();
    DestroyWindow(hwnd);
    UnregisterClass(szWinClass, hThisInstance);
    DeleteObject(hBrush);


    return 0;
}

