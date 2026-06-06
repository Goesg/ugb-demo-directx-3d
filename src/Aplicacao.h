#pragma once
#include "Janela.h"
#include "Renderizador.h"
#include "Camera.h"
#include "Modelo.h"
#include "Textura.h"
#include <memory>
#include <chrono>
#include <vector>
#include <string>

class Aplicacao {
public:
    Aplicacao();
    ~Aplicacao() = default;

    bool inicializar();
    void executar();

private:
    void atualizar(float deltaTempo);
    void renderizar();
    bool carregarTexturas();

    std::unique_ptr<Janela>       janela;
    std::unique_ptr<Renderizador> renderizador;
    std::unique_ptr<Camera>       camera;
    Modelo                        modelo;

    float anguloRotacao = 0.0f;

    std::vector<Textura> texturas;
    int  indiceTextura   = 0;
    bool teclaTPresionada = false;

    DadosLuz luz = {
        XMFLOAT3(0.5f, -0.8f, 0.3f),
        1.0f,
        XMFLOAT3(1.0f, 1.0f, 1.0f),
        0.15f
    };

    static constexpr int LARGURA = 1280;
    static constexpr int ALTURA  = 720;
};
