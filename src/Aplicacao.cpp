#include "Aplicacao.h"

Aplicacao::Aplicacao()
    : janela(std::make_unique<Janela>(LARGURA, ALTURA, L"Demo 3D DirectX 11"))
    , renderizador(std::make_unique<Renderizador>())
    , camera(std::make_unique<Camera>(static_cast<float>(LARGURA), static_cast<float>(ALTURA)))
{
}

bool Aplicacao::inicializar() {
    if (!janela->inicializar()) return false;
    if (!renderizador->inicializar(janela->obterHwnd(), LARGURA, ALTURA)) return false;

    if (!modelo.carregar(renderizador->obterDevice(),
                         renderizador->obterContexto(),
                         "assets/models/modelo.obj",
                         "assets/textures/textura.png")) {
        MessageBoxW(nullptr,
                    L"Falha ao carregar modelo.\n"
                    L"Verifique se existem:\n"
                    L"  assets/models/modelo.obj\n"
                    L"  assets/textures/textura.png",
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
}

void Aplicacao::renderizar() {
    renderizador->limparTela(0.05f, 0.08f, 0.15f);
    renderizador->desenharModelo(
        modelo.obterMalha(),
        modelo.obterTextura(),
        modelo.obterMatrizMundo(),
        camera->obterMatrizVisao(),
        camera->obterMatrizProjecao(),
        luz
    );
    renderizador->apresentar();
}
