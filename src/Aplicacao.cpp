#include "Aplicacao.h"
#include <windows.h>
#include <filesystem>

Aplicacao::Aplicacao()
    : janela(std::make_unique<Janela>(LARGURA, ALTURA, L"Demo 3D DirectX 11"))
    , renderizador(std::make_unique<Renderizador>())
    , camera(std::make_unique<Camera>(static_cast<float>(LARGURA), static_cast<float>(ALTURA)))
{
}

bool Aplicacao::carregarTexturas() {
    namespace fs = std::filesystem;
    const fs::path pastaTexturas = "assets/textures";

    // Coletar caminhos de todas as PNGs em ordem alfabética
    std::vector<std::string> caminhos;
    for (const auto& entrada : fs::directory_iterator(pastaTexturas)) {
        if (entrada.path().extension() == ".png")
            caminhos.push_back(entrada.path().string());
    }
    std::sort(caminhos.begin(), caminhos.end());

    for (const auto& caminho : caminhos) {
        texturas.emplace_back();
        if (!texturas.back().carregar(renderizador->obterDevice(),
                                      renderizador->obterContexto(),
                                      caminho)) {
            texturas.pop_back(); // ignorar texturas que falham ao carregar
        }
    }

    return !texturas.empty();
}

bool Aplicacao::inicializar() {
    if (!janela->inicializar()) return false;
    if (!renderizador->inicializar(janela->obterHwnd(), LARGURA, ALTURA)) return false;

    if (!modelo.carregar(renderizador->obterDevice(),
                         renderizador->obterContexto(),
                         "assets/models/modelo.obj",
                         "")) { // textura gerenciada separadamente
        MessageBoxW(nullptr,
                    L"Falha ao carregar modelo.\n"
                    L"Verifique se existe: assets/models/modelo.obj",
                    L"Erro", MB_OK | MB_ICONWARNING);
        return false;
    }

    if (!carregarTexturas()) {
        MessageBoxW(nullptr,
                    L"Nenhuma textura PNG encontrada em assets/textures/.\n"
                    L"Adicione ao menos um arquivo .png na pasta.",
                    L"Erro", MB_OK | MB_ICONWARNING);
        return false;
    }

    return true;
}

void Aplicacao::executar() {
    using Relogio = std::chrono::high_resolution_clock;
    auto ultimoTempo = Relogio::now();

    while (janela->processarMensagens()) {
        auto agora       = Relogio::now();
        float deltaTempo = std::chrono::duration<float>(agora - ultimoTempo).count();
        ultimoTempo      = agora;

        atualizar(deltaTempo);
        renderizar();
    }
}

void Aplicacao::atualizar(float deltaTempo) {
    camera->processar(deltaTempo);

    anguloRotacao += deltaTempo * 0.5f;
    modelo.definirMatrizMundo(XMMatrixRotationY(anguloRotacao));

    // Alternar textura com T (detectar borda de descida para não pular várias)
    bool tAtiva = (GetAsyncKeyState('T') & 0x8000) != 0;
    if (tAtiva && !teclaTPresionada)
        indiceTextura = (indiceTextura + 1) % static_cast<int>(texturas.size());
    teclaTPresionada = tAtiva;
}

void Aplicacao::renderizar() {
    renderizador->limparTela(0.05f, 0.08f, 0.15f);
    renderizador->desenharModelo(
        modelo.obterMalha(),
        texturas[indiceTextura],
        modelo.obterMatrizMundo(),
        camera->obterMatrizVisao(),
        camera->obterMatrizProjecao(),
        luz
    );
    renderizador->apresentar();
}
