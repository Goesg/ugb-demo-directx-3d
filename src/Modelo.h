#pragma once
#include "Malha.h"
#include "Textura.h"
#include <DirectXMath.h>
#include <string>

using namespace DirectX;

// Carrega um modelo OBJ e sua textura, mantendo a world matrix
class Modelo {
public:
    bool carregar(ID3D11Device* device, ID3D11DeviceContext* contexto,
                  const std::string& caminhoObj,
                  const std::string& caminhoTextura);

    Malha&   obterMalha()    { return malha; }
    Textura& obterTextura()  { return textura; }

    const XMMATRIX& obterMatrizMundo() const { return matrizMundo; }
    void definirMatrizMundo(const XMMATRIX& m) { matrizMundo = m; }

private:
    Malha    malha;
    Textura  textura;
    XMMATRIX matrizMundo = XMMatrixIdentity();
};
