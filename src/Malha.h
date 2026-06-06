#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <vector>
#include "Renderizador.h"

using Microsoft::WRL::ComPtr;

// Encapsula vertex buffer e index buffer de uma malha na GPU
class Malha {
public:
    bool criar(ID3D11Device* device,
               const std::vector<Vertice>& vertices,
               const std::vector<UINT>& indices);

    void bind(ID3D11DeviceContext* contexto) const;

    UINT obterNumIndices() const { return numIndices; }

private:
    ComPtr<ID3D11Buffer> bufferVertices;
    ComPtr<ID3D11Buffer> bufferIndices;
    UINT numIndices = 0;
};
