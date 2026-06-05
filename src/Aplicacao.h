#pragma once
#include "Janela.h"
#include "Renderizador.h"
#include "Camera.h"
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

    XMMATRIX matrizMundo;
    float    anguloRotacao = 0.0f;

    static constexpr int LARGURA = 1280;
    static constexpr int ALTURA  = 720;
};
