#include "Aplicacao.h"
#include <stdexcept>

Aplicacao::Aplicacao()
    : janela(std::make_unique<Janela>(LARGURA, ALTURA, L"Demo 3D DirectX 11"))
    , renderizador(std::make_unique<Renderizador>())
{
}

bool Aplicacao::inicializar() {
    if (!janela->inicializar()) return false;
    if (!renderizador->inicializar(janela->obterHwnd(), LARGURA, ALTURA)) return false;
    return true;
}

void Aplicacao::executar() {
    while (janela->processarMensagens()) {
        atualizar();
        renderizar();
    }
}

void Aplicacao::atualizar() {
    // Etapa 1: sem lógica de atualização ainda
}

void Aplicacao::renderizar() {
    renderizador->limparTela(0.05f, 0.08f, 0.15f);
    renderizador->desenharTriangulo();
    renderizador->apresentar();
}
