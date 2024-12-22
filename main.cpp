#include <windows.h>
#include <gdiplus.h>
#include <iostream>
#include <tlhelp32.h>
#include "heximage.h" //File รูปภาพ มาจาก Hex จากโปรแกรม Hxd
#include "xor.h" // Protect String
#pragma comment (lib, "Gdiplus.lib")
#pragma comment (lib, "Ole32.lib")

using namespace Gdiplus;

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

ULONG_PTR StartGDIPlus() {
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, nullptr);
    return gdiplusToken;
}

void ShutdownGDIPlus(ULONG_PTR gdiplusToken) {
    GdiplusShutdown(gdiplusToken);
}

Bitmap* LoadImageFromHex(const unsigned char* data, size_t size) {
    HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
    if (!hGlobal) return nullptr;

    void* pData = GlobalLock(hGlobal);
    memcpy(pData, data, size);
    GlobalUnlock(hGlobal);

    IStream* pStream = nullptr;
    if (CreateStreamOnHGlobal(hGlobal, TRUE, &pStream) != S_OK) {
        GlobalFree(hGlobal);
        return nullptr;
    }

    Bitmap* bitmap = Bitmap::FromStream(pStream);
    pStream->Release();
    return bitmap;
}

void DetectCheat(HWND hWnd) {
    MessageBox(hWnd, "Cheat detected!", "Warning", MB_OK | MB_ICONEXCLAMATION);
    PostQuitMessage(0);  
}

BOOL DetectByTitle(HWND hWnd) {
    HWND window;

 
    window = FindWindow(0, xorstr_("IDA: Quick start"));
    if (window) {
        DetectCheat(hWnd);
        return TRUE;
    }
    window = FindWindow(0, xorstr_("Memory Viewer"));
    if (window) {
        DetectCheat(hWnd);
        return TRUE;
    }
    window = FindWindow(0, xorstr_("Process List"));
    if (window) {
        DetectCheat(hWnd);
        return TRUE;
    }
    window = FindWindow(0, xorstr_("Cheat Engine"));
    if (window) {
        DetectCheat(hWnd);
        return TRUE;
    }
    window = FindWindow(0, xorstr_("Process Hacker"));
    if (window) {
        DetectCheat(hWnd);
        return TRUE;
    }
    window = FindWindow(0, xorstr_("KK            "));
    if (window) {
        DetectCheat(hWnd);
        return TRUE;
    }
    window = FindWindow(0, xorstr_("FIREZ          "));
    if (window) {
        DetectCheat(hWnd);
        return TRUE;
    }
    //สามารถเพิ่ม Title ที่ต้องการ Detect ได้ตรงนี้
    return FALSE;  
}
bool IsCheatProcessRunning(HWND hWnd) {
    const char* cheatProcesses[] = {
        "cheatengine.exe",
        "ida64.exe",
        "processhacker.exe" //สามารถเพิ่มโปรแกรมที่ต้องการ Detect ได้ตรงนี้
    };

    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        return false;
    }

    PROCESSENTRY32 pe32;
    pe32.dwSize = sizeof(PROCESSENTRY32);

    if (!Process32First(hSnapshot, &pe32)) {
        DetectCheat(hWnd);
        CloseHandle(hSnapshot);
        return false;
    }

    do {
        for (const char* cheatProcess : cheatProcesses) {
            if (strstr(pe32.szExeFile, cheatProcess) != nullptr) {
                CloseHandle(hSnapshot);
                return true; 
            }
        }
    } while (Process32Next(hSnapshot, &pe32));

    CloseHandle(hSnapshot);
    return false;
}
bool IsDebuggerPresentEx() {
    return IsDebuggerPresent() || GetModuleHandle("dbghelp.dll") != nullptr;
}
int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow) {
    ULONG_PTR gdiplusToken = StartGDIPlus();

    const char CLASS_NAME[] = "ImageWindowClass";
    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;
    RegisterClass(&wc);

    HWND hWnd = CreateWindowEx(
        WS_EX_TOPMOST, CLASS_NAME, nullptr, WS_POPUP,
        0, 0, 600, 350, nullptr, nullptr, hInstance, nullptr);

    if (!hWnd) return 0;

    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);
    SetWindowPos(hWnd, nullptr, (screenWidth - 600) / 2, (screenHeight - 350) / 2, 600, 350, SWP_SHOWWINDOW);

    ShowWindow(hWnd, SW_SHOW);
    UpdateWindow(hWnd);

    MSG msg = {};
    while (GetMessage(&msg, nullptr, 0, 0)) {
        if (DetectByTitle(hWnd)) {
           
            break;
        }
        if (IsCheatProcessRunning(hWnd)) {
            break;
        }
        if (IsDebuggerPresentEx()) {
            DetectCheat(hWnd);
            break;
        }

        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    ShutdownGDIPlus(gdiplusToken);
    return 0;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    static Bitmap* image = nullptr;
    static RECT closeButton = { 570, 10, 590, 30 };
    switch (message) {
    case WM_CREATE:
        image = LoadImageFromHex(imageData, sizeof(imageData));
        if (!image) {
            PostQuitMessage(0);
        }
        break;

    case WM_PAINT: {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        Graphics graphics(hdc);
        if (image) {
            graphics.DrawImage(image, 0, 0, 600, 350);
        }

        SolidBrush redBrush(Color(255, 255, 0, 0));
        Pen whitePen(Color(255, 255, 255), 2);

        int centerX = closeButton.left + 10;
        int centerY = closeButton.top + 10;
        int radius = 8;

        graphics.FillEllipse(&redBrush,
            static_cast<REAL>(centerX - radius),
            static_cast<REAL>(centerY - radius),
            static_cast<REAL>(radius * 2),
            static_cast<REAL>(radius * 2));

        int offset = 4;
        graphics.DrawLine(&whitePen,
            static_cast<REAL>(centerX - offset), static_cast<REAL>(centerY - offset),
            static_cast<REAL>(centerX + offset), static_cast<REAL>(centerY + offset));

        graphics.DrawLine(&whitePen,
            static_cast<REAL>(centerX + offset), static_cast<REAL>(centerY - offset),
            static_cast<REAL>(centerX - offset), static_cast<REAL>(centerY + offset));

        EndPaint(hWnd, &ps);
        break;
    }

    case WM_SETCURSOR:
        SetCursor(LoadCursor(NULL, IDC_ARROW));
        return TRUE;

    case WM_LBUTTONDOWN: {
        int xPos = LOWORD(lParam);
        int yPos = HIWORD(lParam);

        int centerX = closeButton.left + 10;
        int centerY = closeButton.top + 10;
        int radius = 8;

        int dx = xPos - centerX;
        int dy = yPos - centerY;
        if ((dx * dx + dy * dy) <= (radius * radius)) {
            PostQuitMessage(0);
        }
        break;
    }

    case WM_DESTROY:
        delete image;
        PostQuitMessage(0);
        break;

    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
