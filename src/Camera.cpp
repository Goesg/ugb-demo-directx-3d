#include "Camera.h"
#include <cmath>

Camera::Camera(float largura, float altura) {
    float aspectRatio = largura / altura;
    matrizProjecao = XMMatrixPerspectiveFovLH(FOV, aspectRatio, 0.1f, 100.0f);
    recalcularDirecao();
}

void Camera::processar(float deltaTempo) {
    // --- Rotação pelas setas do teclado ---
    if (GetAsyncKeyState(VK_LEFT)  & 0x8000) yaw   -= sensibilidade * deltaTempo;
    if (GetAsyncKeyState(VK_RIGHT) & 0x8000) yaw   += sensibilidade * deltaTempo;
    if (GetAsyncKeyState(VK_UP)    & 0x8000) pitch += sensibilidade * deltaTempo;
    if (GetAsyncKeyState(VK_DOWN)  & 0x8000) pitch -= sensibilidade * deltaTempo;

    // Limitar pitch para evitar que a câmera vire de cabeça para baixo
    if (pitch >  PITCH_MAX) pitch =  PITCH_MAX;
    if (pitch < -PITCH_MAX) pitch = -PITCH_MAX;

    recalcularDirecao();

    // --- Movimentação WASD ---
    // Calcular vetores de movimento a partir da direção atual
    XMVECTOR frente = XMLoadFloat3(&direcao);
    XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    XMVECTOR direita = XMVector3Normalize(XMVector3Cross(frente, up));

    float vel = velocidade * deltaTempo;

    XMVECTOR pos = XMLoadFloat3(&posicao);
    if (GetAsyncKeyState('W') & 0x8000) pos = XMVectorAdd(pos, XMVectorScale(frente,  vel));
    if (GetAsyncKeyState('S') & 0x8000) pos = XMVectorAdd(pos, XMVectorScale(frente, -vel));
    if (GetAsyncKeyState('D') & 0x8000) pos = XMVectorAdd(pos, XMVectorScale(direita,  vel));
    if (GetAsyncKeyState('A') & 0x8000) pos = XMVectorAdd(pos, XMVectorScale(direita, -vel));

    XMStoreFloat3(&posicao, pos);
}

XMMATRIX Camera::obterMatrizVisao() const {
    XMVECTOR pos    = XMLoadFloat3(&posicao);
    XMVECTOR dir    = XMLoadFloat3(&direcao);
    XMVECTOR alvo   = XMVectorAdd(pos, dir);
    XMVECTOR up     = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
    return XMMatrixLookAtLH(pos, alvo, up);
}

void Camera::recalcularDirecao() {
    // Converter yaw/pitch em vetor direção (esfera unitária)
    direcao.x = std::cos(pitch) * std::sin(yaw);
    direcao.y = std::sin(pitch);
    direcao.z = std::cos(pitch) * std::cos(yaw);
}
