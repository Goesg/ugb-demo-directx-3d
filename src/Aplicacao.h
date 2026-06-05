#pragma once
#include "Janela.h"
#include "Renderizador.h"
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

    // Matrizes da cena
    XMMATRIX matrizMundo;
    XMMATRIX matrizVisao;
    XMMATRIX matrizProjecao;

    float anguloRotacao = 0.0f;

    static constexpr int   LARGURA = 1280;
    static constexpr int   ALTURA  = 720;
    static constexpr float FOV     = XM_PIDIV4; // 45 graus em radianos
};
