#pragma once
#include "Janela.h"
#include "Renderizador.h"
#include <memory>

// Ponto central: inicializa módulos e executa o loop principal
class Aplicacao {
public:
    Aplicacao();
    ~Aplicacao() = default;

    bool inicializar();
    void executar();

private:
    void atualizar();
    void renderizar();

    std::unique_ptr<Janela>       janela;
    std::unique_ptr<Renderizador> renderizador;

    static constexpr int LARGURA = 1280;
    static constexpr int ALTURA  = 720;
};
