#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <string>

// Gerencia a janela Win32: criação, mensagens e ciclo de vida
class Janela {
public:
    Janela(int largura, int altura, const std::wstring& titulo);
    ~Janela();

    bool inicializar();
    bool processarMensagens();

    HWND obterHwnd() const { return hwnd; }
    int  obterLargura() const { return largura; }
    int  obterAltura() const { return altura; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    HWND         hwnd    = nullptr;
    HINSTANCE    hInst   = nullptr;
    std::wstring titulo;
    int          largura = 0;
    int          altura  = 0;
};
