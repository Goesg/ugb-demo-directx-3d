#include "Aplicacao.h"
#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Aplicacao app;

    if (!app.inicializar()) {
        MessageBoxW(nullptr, L"Falha ao inicializar a aplicação.", L"Erro", MB_OK | MB_ICONERROR);
        return -1;
    }

    app.executar();
    return 0;
}
