// main.cpp
#include <windows.h>
#include <gdiplus.h>
#include <vector>
#include <string>
#include "logikaWindy.h"
#pragma comment(lib, "gdiplus.lib")

using namespace Gdiplus;
using namespace std;

#define BUTTON_ID_BASE 1000

Winda winda;
vector<Pasazer> pasazerowie;



void DrawWinda(Graphics& g)
{
    SolidBrush windaBrush(Color(150, 150, 255));
    SolidBrush pasazerBrush(Color(0, 180, 0));
    SolidBrush czekaBrush(Color(255, 100, 100));
    SolidBrush dojechalBrush(Color(100, 100, 255));

    FontFamily fontFamily(L"Arial");
    Font font(&fontFamily, 12, FontStyleRegular, UnitPixel);
    SolidBrush tekstBrush(Color(0, 0, 0));
    Pen pen(Color(0, 0, 0));

    // Rozmiary


    int marginY = 300; // pi�tro 0
    int wysokoscPietro = 50;
    int szerokoscWindy = 125;
    int wysokoscWindy = 50;
    int szerokoscPasazer = 10;
    int wysokoscPasazer = 20;
    int marginX = 50;
    int czekajacyX = marginX + 80;
    int windaX = czekajacyX + 100;
    int dojechaliX = windaX + szerokoscWindy + 30;
 
    

    // Waga
    wstring wagaStr = L"Waga: " + to_wstring(winda.waga) + L" kg";
    g.DrawString(wagaStr.c_str(), -1, &font, PointF(marginX, 10), &tekstBrush);

    // Rysowanie pi�ter od 0 do 4
    for (int f = 0; f < 5; ++f) {
        int fy = marginY - f * wysokoscPietro;

        // Linia pozioma pi�tra
        g.DrawLine(&pen, marginX, fy, dojechaliX + 100, fy);

        // Nazwa pi�tra w lewym g�rnym rogu pi�tra
        wstring pietroText = L"Pietro " + to_wstring(f);
        g.DrawString(pietroText.c_str(), -1, &font, PointF(marginX, fy - wysokoscPietro + 5), &tekstBrush);

        // Pasa�erowie czekaj�cy
        int count = 0;
        for (const Pasazer& p : pasazerowie) {
            if (p.stan == czeka && p.pietroStart == f) {
                int px = czekajacyX + (count % 8) * (szerokoscPasazer + 2);
                int py = fy - wysokoscPasazer - 5 - (count / 8) * (wysokoscPasazer + 2);
                g.FillRectangle(&czekaBrush, px, py, szerokoscPasazer, wysokoscPasazer);
                count++;
            }
        }

        // Pasa�erowie, kt�rzy dojechali
        count = 0;
        for (const Pasazer& p : pasazerowie) {
            if (p.stan == dojechal && p.pietroKoniec == f) {
                int px = dojechaliX + (count % 8) * (szerokoscPasazer + 2);
                int py = fy - wysokoscPasazer - 5 - (count / 8) * (wysokoscPasazer + 2);
                g.FillRectangle(&dojechalBrush, px, py, szerokoscPasazer, wysokoscPasazer);
                count++;
            }
        }
    }

    // Winda � rysowana raz
    int wy = marginY - ((winda.pietro + 1) * wysokoscPietro);
    g.FillRectangle(&windaBrush, windaX, wy, szerokoscWindy, wysokoscWindy);

    // Pasa�erowie w windzie
    int i = 0;
    for (auto* p : winda.vectorPasazerow) {
        int px = windaX + 5 + (i % 8) * (szerokoscPasazer + 2);
        int py = wy + wysokoscWindy - wysokoscPasazer - 5 - (i / 8) * (wysokoscPasazer + 2);
        g.FillRectangle(&pasazerBrush, px, py, szerokoscPasazer, wysokoscPasazer);
        i++;
    }
}




LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE:
        SetTimer(hWnd, 1, 500, NULL);

        for (int start = 0; start < 5; ++start) {
            for (int cel = 0; cel < 5; ++cel) {
                if (cel == start) continue;
                wchar_t label[16];
                swprintf(label, 16, L"Do %d", cel);
                int btnId = BUTTON_ID_BASE + start * 10 + cel;

                CreateWindow(L"BUTTON", label,
                    WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
                    500+cel * 45, // x
                    300 - start * 50 -30, // y
                    40, 20, hWnd, (HMENU)btnId, NULL, NULL);
            }
        }
        return 0;

    case WM_COMMAND:
    {
        int btnId = LOWORD(wParam);
        if (btnId >= BUTTON_ID_BASE && btnId < BUTTON_ID_BASE + 500) {
            int floor = (btnId - BUTTON_ID_BASE) / 10;
            int cel = (btnId - BUTTON_ID_BASE) % 10;
            //if (!((/*floor==winda.pietro - 1||*/ floor == winda.pietro + 1 || floor == winda.pietro) && winda.kierunek==gora)){
                //if (!((floor == winda.pietro - 1 /*|| floor == winda.pietro + 1*/ || floor == winda.pietro) && winda.kierunek == dol)) {
                     pasazerowie.push_back(Pasazer(floor, cel));
                //}

            //}
            InvalidateRect(hWnd, NULL, FALSE);
        }
        return 0;
    }

    case WM_TIMER:
        for (Pasazer& p : pasazerowie) winda.wezwij(p);
        for (Pasazer& p : pasazerowie) winda.odbierz(p);
        winda.ruch();
        winda.odstaw();
        winda.pierwszyRuchJe�liPotrzeba();
        InvalidateRect(hWnd, NULL, FALSE);
        return 0;

    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        {
            Graphics g(hdc);
            RECT winRect;
            GetClientRect(hWnd, &winRect);
            int width = winRect.right - winRect.left;
            int height = winRect.bottom - winRect.top;
            SolidBrush whiteBrush(Color(255, 255, 255));
            g.FillRectangle(&whiteBrush, Rect(0, 0, width, height));
            DrawWinda(g);
        }
        EndPaint(hWnd, &ps);
    }
    return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hWnd, message, wParam, lParam);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE, LPSTR, int nCmdShow)
{
    GdiplusStartupInput gdiplusStartupInput;
    ULONG_PTR gdiplusToken;
    GdiplusStartup(&gdiplusToken, &gdiplusStartupInput, NULL);

    WNDCLASS wc = {};
    wc.lpfnWndProc = WndProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = L"SymulacjaWindy";

    RegisterClass(&wc);
    HWND hWnd = CreateWindowEx(WS_EX_COMPOSITED, L"SymulacjaWindy", L"Symulator Windy", WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 400, NULL, NULL, hInstance, NULL);

    ShowWindow(hWnd, nCmdShow);

    MSG msg = {};
    while (GetMessage(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    GdiplusShutdown(gdiplusToken);
    return 0;
}