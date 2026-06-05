#pragma once
#include <DirectXMath.h>
#include <windows.h>

using namespace DirectX;

// Câmera FPS com movimentação WASD e rotação pelas setas do teclado
class Camera {
public:
    Camera(float largura, float altura);

    void processar(float deltaTempo);

    XMMATRIX obterMatrizVisao()    const;
    XMMATRIX obterMatrizProjecao() const { return matrizProjecao; }

private:
    void recalcularDirecao();

    XMFLOAT3 posicao  = { 0.0f, 1.0f, -4.0f };
    XMFLOAT3 direcao  = { 0.0f, 0.0f,  1.0f };

    float yaw         =  0.0f;          // rotação horizontal (eixo Y)
    float pitch       =  0.0f;          // rotação vertical (eixo X)
    float velocidade  =  3.0f;          // unidades por segundo
    float sensibilidade = 1.2f;         // radianos por segundo ao segurar a seta

    XMMATRIX matrizProjecao;

    static constexpr float PITCH_MAX = XM_PIDIV2 - 0.01f; // ~89 graus
    static constexpr float FOV       = XM_PIDIV4;          // 45 graus
};
