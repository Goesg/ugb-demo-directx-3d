#include "Aplicacao.h"

Aplicacao::Aplicacao()
    : janela(std::make_unique<Janela>(LARGURA, ALTURA, L"Demo 3D DirectX 11"))
    , renderizador(std::make_unique<Renderizador>())
{
}

bool Aplicacao::inicializar() {
    if (!janela->inicializar())     return false;
    if (!renderizador->inicializar(janela->obterHwnd(), LARGURA, ALTURA)) return false;

    // Câmera fixa: posicionada atrás e acima olhando para a origem
    matrizVisao = XMMatrixLookAtLH(
        XMVectorSet(0.0f, 1.5f, -4.0f, 0.0f), // posição da câmera
        XMVectorSet(0.0f, 0.0f,  0.0f, 0.0f), // ponto alvo
        XMVectorSet(0.0f, 1.0f,  0.0f, 0.0f)  // vetor up
    );

    // Projeção perspectiva: FOV 45°, aspect ratio da janela, near=0.1, far=100
    float aspectRatio = static_cast<float>(LARGURA) / static_cast<float>(ALTURA);
    matrizProjecao = XMMatrixPerspectiveFovLH(FOV, aspectRatio, 0.1f, 100.0f);

    return true;
}

void Aplicacao::executar() {
    using Relogio = std::chrono::high_resolution_clock;
    auto ultimoTempo = Relogio::now();

    while (janela->processarMensagens()) {
        auto agora      = Relogio::now();
        float deltaTempo = std::chrono::duration<float>(agora - ultimoTempo).count();
        ultimoTempo     = agora;

        atualizar(deltaTempo);
        renderizar();
    }
}

void Aplicacao::atualizar(float deltaTempo) {
    // Rotacionar o cubo continuamente em torno dos eixos Y e X
    anguloRotacao += deltaTempo * 1.0f; // 1 radiano por segundo

    matrizMundo = XMMatrixRotationY(anguloRotacao) *
                  XMMatrixRotationX(anguloRotacao * 0.5f);
}

void Aplicacao::renderizar() {
    renderizador->limparTela(0.05f, 0.08f, 0.15f);
    renderizador->desenharCubo(matrizMundo, matrizVisao, matrizProjecao);
    renderizador->apresentar();
}
