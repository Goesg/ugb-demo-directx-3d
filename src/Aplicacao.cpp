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

    if (!textura.carregar(renderizador->obterDevice(),
                          renderizador->obterContexto(),
                          "assets/textures/textura.png")) {
        MessageBoxW(nullptr, L"Falha ao carregar textura.\nVerifique se assets/textures/textura.png existe.",
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
    matrizMundo = XMMatrixRotationY(anguloRotacao);
}

void Aplicacao::renderizar() {
    renderizador->limparTela(0.05f, 0.08f, 0.15f);
    renderizador->desenharCubo(matrizMundo,
                                camera->obterMatrizVisao(),
                                camera->obterMatrizProjecao(),
                                textura, luz);
    renderizador->apresentar();
}
