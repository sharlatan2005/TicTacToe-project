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

struct POINT_COUNTER {
    int w_points = 0;
    int h_points = 0;
    int d_points1 = 0;
    int d_points2 = 0;
};

COLOR bgColor = {0, 0 , 255}; // цвет фона окна
COLOR cellColor = {255, 0, 0}; // цвет сетки
long matrixSize = 5; // N
long wWidth = 320; // ширина окна
long wHeight = 240; // высота окна
LPTSTR buf;
UINT synchMessage;
UINT endOfGame;
int turn_counter = 0; // при turn_counter == matrixSize * matrixSize игра завершается
int whose_turn = 0;
bool thread_is_suspended = 0;

HANDLE hThread;

enum cChanges {
    red_up,
    red_down,
    green_up,
    green_down,
    blue_up,
    blue_down
};

cChanges state = green_up;
cChanges state_bg = red_up;
const TCHAR szWinClass[] = _T("Win32SampleApp");
const TCHAR szWinName[] = _T("Win32SampleWindow");
const TCHAR szSharedMemoryName[] = _T("SharedMemory31233123");
HWND hwnd;               /* This is the handle for our window */
HBRUSH hBrush;           /* Current brush */



void StateSetter(cChanges& state, const COLOR& color) {
    if (color.r == 255) {
        if (color.b == 0) {
            state = green_up;
        }
        if (color.g == 0) {
            state = blue_down;
        }
    }
    if (color.g == 255) {
        if (color.r == 0) {
            state = blue_up;
        }
        if (color.b == 0) {
            state = red_down;
        }
    }
    if (color.b == 255) {
        if (color.r == 0) {
            state = green_down;
        }
        if (color.g == 0) {
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

bool WinChecker(int x, int y, int figure) {
    POINT_COUNTER counter;
    for (int i = 0; i < matrixSize; i++) {
        if (buf[x / (wWidth / matrixSize) * matrixSize + i] == figure) {
            counter.w_points++;
        }
    } // проверка ширины
    for (int i = 0; i < matrixSize; i++) {
        if (buf[y / (wHeight / matrixSize) + (matrixSize * i)] == figure) {
            counter.h_points++;
        }
    } // проверка длины
    for (int i = 0; i < matrixSize; i++) {
        if (buf[i * matrixSize + i] == figure) {
            counter.d_points1++;
        }
        if (buf[i * matrixSize + matrixSize - i - 1] == figure) {
            counter.d_points2++;
        } // проверка обоих диагоналей
    }
    if (counter.w_points == matrixSize || counter.h_points == matrixSize || counter.d_points1 == matrixSize || counter.d_points2 == matrixSize) {
        return true;
    }
    return false;
}

void SwitchBgColor() {
    switch (state_bg) {
    case(green_up):
        bgColor.g += 5;
        if (bgColor.b == 255) {
            state_bg = red_down;
        }
        break;
    case(red_down):
        bgColor.r -= 5;
        if (bgColor.r == 0) {
            state_bg = blue_up;
        }
        break;
    case(blue_up):
        bgColor.b += 5;
        if (bgColor.b == 255) {
            state_bg = green_down;
        }
        break;
    case(green_down):
        bgColor.g -= 5;
        if (bgColor.g == 0) {
            state_bg = red_up;
        }
        break;
    case(red_up):
        bgColor.r += 5;
        if (bgColor.r == 255) {
            state_bg = blue_down;
        }
        break;
    case(blue_down):
        bgColor.b -= 5;
        if (bgColor.b == 0) {
            state_bg = green_up;
        }
    break;
    }
    hBrush = CreateSolidBrush(RGB(bgColor.r, bgColor.g, bgColor.b));
    HBRUSH hTempBrush = (HBRUSH)(DWORD_PTR)SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG)hBrush);
    DeleteObject(hTempBrush);
}

DWORD WINAPI ThreadFunc(LPVOID) {

    HDC hdc;
    PAINTSTRUCT ps;

    while (TRUE) {
        hdc = BeginPaint(hwnd, &ps);
        SwitchBgColor();
        //Sleep(100);
        RECT wSize;
        GetClientRect(hwnd, &wSize);
        FillRect(hdc, &wSize, hBrush);
        HPEN hPen = CreatePen(PS_SOLID, 2, COLORREF(RGB(cellColor.r, cellColor.g, cellColor.b)));
        HPEN hDefaultPen = (HPEN)SelectObject(hdc, hPen);
        for (long i = 0; i < matrixSize; i++) {
            MoveToEx(hdc, i * wWidth / matrixSize, 0, NULL); // wSize.right = wWidth; wSize.bottpm = wHeight;
            LineTo(hdc, i * wWidth / matrixSize, wHeight);
            MoveToEx(hdc, 0, i * wHeight / matrixSize, NULL);
            LineTo(hdc, wWidth, i * wHeight / matrixSize);
        }
        hPen = (HPEN)SelectObject(hdc, hDefaultPen);
        DeleteObject(hPen);

        hPen = CreatePen(PS_SOLID, 2, COLORREF(RGB(0, 0, 0)));
        hDefaultPen = (HPEN)SelectObject(hdc, hPen);

        HBRUSH hDefaultBrush = (HBRUSH)SelectObject(hdc, hBrush);
        for (int i = 0; i < matrixSize; i++) {
            for (int j = 0; j < matrixSize; j++) {
                if (buf[i * matrixSize + j] == 1) {
                    Ellipse(hdc, i * wWidth / matrixSize, j * wHeight / matrixSize, (i + 1) * wWidth / matrixSize, (j + 1) * wHeight / matrixSize);
                }
                if (buf[i * matrixSize + j] == 2) {
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
        RedrawWindow(hwnd, &wSize, NULL, RDW_INVALIDATE);
    }
    
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
        if (whose_turn != 2) {
            x = GET_X_LPARAM(lParam);
            y = GET_Y_LPARAM(lParam);
            if (buf[(x / (wWidth / matrixSize)) * matrixSize + y / (wHeight / matrixSize)] == 0) {
                buf[(x / (wWidth / matrixSize)) * matrixSize + y / (wHeight / matrixSize)] = 1;
                whose_turn = 2;
                if (WinChecker(x, y, 1)) {
                    PostMessage(HWND_BROADCAST, endOfGame, NULL, 1);
                    /*MessageBox(hwnd, L"Выиграли нолики", L"Конец игры", MB_OK);
                    TerminateThread(hThread, 0);
                    PostQuitMessage(0);*/
                }
                if (++turn_counter == matrixSize * matrixSize) {
                    PostMessage(HWND_BROADCAST, endOfGame, NULL, 3);
                    /*MessageBox(hwnd, L"Ничья", L"Конец игры", MB_OK);
                    TerminateThread(hThread, 0);
                    PostQuitMessage(0);*/
                }
            }
        }
        else {
            MessageBox(hwnd, L"Сейчас ходят крестики", L"Ошибка!", MB_OK);
        }
        /*SendMessage(HWND_BROADCAST, synchMessage, NULL, NULL);*/
        return 0;
    }
    case WM_RBUTTONUP:
    {
        if (whose_turn != 1) {
            x = GET_X_LPARAM(lParam);
            y = GET_Y_LPARAM(lParam);
            if (buf[(x / (wWidth / matrixSize)) * matrixSize + y / (wHeight / matrixSize)] == 0) {
                buf[(x / (wWidth / matrixSize)) * matrixSize + y / (wHeight / matrixSize)] = 2;
                whose_turn = 1;
                if (WinChecker(x, y, 2)) {
                    PostMessage(HWND_BROADCAST, endOfGame, NULL, 2);
                    /*MessageBox(hwnd, L"Выиграли крестики", L"Конец игры", MB_OK);
                    TerminateThread(hThread, 0);
                    PostQuitMessage(0);*/
                }
                if (++turn_counter == matrixSize * matrixSize) {
                    PostMessage(HWND_BROADCAST, endOfGame, NULL, 3);
                    /*MessageBox(hwnd, L"Ничья", L"Конец игры", MB_OK);
                    TerminateThread(hThread, 0);
                    PostQuitMessage(0);*/
                }
            }
        }
        else {
            MessageBox(hwnd, L"Сейчас ходят нолики", L"Ошибка!", MB_OK);
        }
        /*SendMessage(HWND_BROADCAST, synchMessage, NULL, NULL);*/
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
        /*InvalidateRect(hwnd, NULL, TRUE);*/
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
            if (GetKeyState(VK_CONTROL) & KEY_SHIFTED) {
                TerminateThread(hThread, 0);
                PostQuitMessage(0);
            }
            return 0;
        case VK_ESCAPE:
            TerminateThread(hThread, 0);
            PostQuitMessage(0);
            return 0;
        case VK_RETURN:
            bgColor = { (uint8_t)(rand() % 256), (uint8_t)(rand() % 256), (uint8_t)(rand() % 256) };
            hBrush = CreateSolidBrush(RGB(bgColor.r, bgColor.g, bgColor.b));
            hTempBrush = (HBRUSH)(DWORD_PTR)SetClassLongPtr(hwnd, GCLP_HBRBACKGROUND, (LONG)hBrush);
            DeleteObject(hTempBrush);
            /*InvalidateRect(hwnd, NULL, TRUE);*/
            return 0;
        case VK_SPACE: 
            if (thread_is_suspended) {
                ResumeThread(hThread);
                thread_is_suspended = false;
            }
            else {
                SuspendThread(hThread);
                thread_is_suspended = true;
            }
            return 0;
        case 48:
            SetThreadPriority(hThread, THREAD_PRIORITY_IDLE);
        case 49:
            SetThreadPriority(hThread, THREAD_PRIORITY_LOWEST);
        case 50:
            SetThreadPriority(hThread, THREAD_PRIORITY_BELOW_NORMAL);
        case 51:
            SetThreadPriority(hThread, THREAD_PRIORITY_NORMAL);
        case 52:
            SetThreadPriority(hThread, THREAD_PRIORITY_ABOVE_NORMAL);
        case 53:
            SetThreadPriority(hThread, THREAD_PRIORITY_HIGHEST);
        case 54:
            SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);
        }

    case WM_SIZE:
    {
        RECT rect;
        GetClientRect(hwnd, &rect);
        wWidth = rect.right;
        wHeight = rect.bottom;
        /*InvalidateRect(hwnd, NULL, TRUE);*/
        return 0;
    }
    case WM_CLOSE:
        /*RECT rect;
        GetClientRect(hwnd, &rect);
        wWidth = rect.right;
        wHeight = rect.bottom;*/
        TerminateThread(hThread, 0);
        PostQuitMessage(0);
        return 0;
    //case WM_PAINT:
    //{
    //    hdc = BeginPaint(hwnd, &ps);
    //    RECT wSize;
    //    GetClientRect(hwnd, &wSize);
    //    FillRect(hdc, &wSize, hBrush);
    //    HPEN hPen = CreatePen(PS_SOLID, NULL, COLORREF(RGB(cellColor.r, cellColor.g, cellColor.b)));
    //    HPEN hDefaultPen = (HPEN)SelectObject(hdc, hPen);
    //    for (long i = 0; i < matrixSize; i++) {
    //        MoveToEx(hdc, i * wWidth / matrixSize, 0, NULL); // wSize.right = wWidth; wSize.bottpm = wHeight;
    //        LineTo(hdc, i * wWidth / matrixSize, wHeight);
    //        MoveToEx(hdc, 0, i * wHeight / matrixSize, NULL);
    //        LineTo(hdc, wWidth, i * wHeight / matrixSize);
    //    }
    //    hPen = (HPEN)SelectObject(hdc, hDefaultPen);
    //    DeleteObject(hPen);

    //    hPen = CreatePen(PS_SOLID, NULL, COLORREF(RGB(0, 0, 0)));
    //    hDefaultPen = (HPEN)SelectObject(hdc, hPen);

    //    HBRUSH hDefaultBrush = (HBRUSH)SelectObject(hdc, hBrush);
    //    for (int i = 0; i < matrixSize; i++) {
    //        for (int j = 0; j < matrixSize; j++) {
    //            if (buf[i * matrixSize + j] == 1) {
    //                Ellipse(hdc, i * wWidth / matrixSize, j * wHeight / matrixSize, (i + 1) * wWidth / matrixSize, (j + 1) * wHeight / matrixSize);
    //            }
    //            if (buf[i * matrixSize + j] == 2) {
    //                MoveToEx(hdc, i * wWidth / matrixSize, j * wHeight / matrixSize, NULL);
    //                LineTo(hdc, (i + 1) * wWidth / matrixSize, (j + 1) * wHeight / matrixSize);
    //                MoveToEx(hdc, i * wWidth / matrixSize, (j + 1) * wHeight / matrixSize, NULL);
    //                LineTo(hdc, (i + 1) * wWidth / matrixSize, j * wHeight / matrixSize);
    //            }
    //        }
    //    }


    //    DeleteObject(hPen);
    //    DeleteObject(hDefaultBrush);

    //    EndPaint(hwnd, &ps);
    //    return 0;
    //}
    default:
        if (message == endOfGame) {
            switch (lParam) {
            case (1):

                MessageBox(hwnd, L"Выиграли нолики", L"Конец игры", MB_OK);
                TerminateThread(hThread, 0);
                PostQuitMessage(0);

            case (2):
                MessageBox(hwnd, L"Выиграли крестики", L"Конец игры", MB_OK);
                TerminateThread(hThread, 0);
                PostQuitMessage(0);
            case (3):
                MessageBox(hwnd, L"Ничья", L"Конец игры", MB_OK);
                TerminateThread(hThread, 0);
                PostQuitMessage(0);
            break;
            }
            return 0;
        }

    break;
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

    StateSetter(state, cellColor);
    StateSetter(state_bg, bgColor);

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
    bool isFirst = 0;
    HANDLE hFileMapping = OpenFileMapping(PAGE_READWRITE, FALSE, szSharedMemoryName);
    if (hFileMapping == NULL) {
        if (argc > 1) {
            matrixSize = atoi(argv[1]);
            std::ofstream fout;
            fout.open("cfg.txt", std::ofstream::out | std::ofstream::trunc);
            fout << matrixSize << "\n" << wWidth << "\n" << wHeight << "\n"
                << bgColor.r << "\n" << bgColor.g << "\n" << bgColor.b << "\n"
                << cellColor.r << "\n" << cellColor.g << "\n" << cellColor.b << "\n";

            fout.close();
        }
    }
    else {
        CloseHandle(hFileMapping);
    }
    
    

    hFileMapping = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, matrixSize * matrixSize, szSharedMemoryName);
    buf = (LPTSTR)MapViewOfFile(hFileMapping, FILE_MAP_ALL_ACCESS, 0, 0, matrixSize * matrixSize);
    
    synchMessage = RegisterWindowMessage((LPCTSTR)_T("Aboba"));

    endOfGame = RegisterWindowMessage((LPCTSTR)_T("AAAAAAAAAA"));
    ShowWindow(hwnd, nCmdShow);

    hThread = CreateThread(NULL, 0, ThreadFunc, NULL, 0, 0);

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
    
    std::ofstream fout;
    fout.open("cfg.txt", std::ofstream::out | std::ofstream::trunc);
    fout << matrixSize << "\n" << wWidth << "\n" << wHeight << "\n"
        << bgColor.r << "\n" << bgColor.g << "\n" << bgColor.b << "\n"
        << cellColor.r << "\n" << cellColor.g << "\n" << cellColor.b << "\n";
        
    fout.close();

    UnmapViewOfFile(buf);
    CloseHandle(hFileMapping);

    DestroyWindow(hwnd);
    UnregisterClass(szWinClass, hThisInstance);
    DeleteObject(hBrush);


    return 0;
}

