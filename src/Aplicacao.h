#pragma once
#include "Janela.h"
#include "Renderizador.h"
#include "Camera.h"
#include "Textura.h"
#include <memory>
#include <chrono>

class Aplicacao {
public:
    Aplicacao();
    ~Aplicacao() = default;

    bool inicializar();
    void executar();

private:
    void atualizar(float deltaTempo);
    void renderizar();

    std::unique_ptr<Janela>       janela;
    std::unique_ptr<Renderizador> renderizador;
    std::unique_ptr<Camera>       camera;
    Textura                       textura;

    XMMATRIX matrizMundo;
    float    anguloRotacao = 0.0f;

    // Luz direcional apontando para baixo e para frente (diagonal)
    DadosLuz luz = {
        XMFLOAT3(0.5f, -0.8f, 0.3f), // direcaoLuz (normalizada no shader)
        1.0f,                          // intensidade
        XMFLOAT3(1.0f, 1.0f, 1.0f),  // corLuz (branca)
        0.15f                          // ambiente (15% mínimo)
    };

    static constexpr int LARGURA = 1280;
    static constexpr int ALTURA  = 720;
};
