#include "Malha.h"

bool Malha::criar(ID3D11Device* device,
                  const std::vector<Vertice>& vertices,
                  const std::vector<UINT>& indices) {
    numIndices = static_cast<UINT>(indices.size());

    D3D11_BUFFER_DESC bd = {};
    bd.Usage     = D3D11_USAGE_IMMUTABLE;
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = static_cast<UINT>(sizeof(Vertice) * vertices.size());

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = vertices.data();
    HRESULT hr = device->CreateBuffer(&bd, &sd, bufferVertices.GetAddressOf());
    if (FAILED(hr)) return false;

    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    bd.ByteWidth = static_cast<UINT>(sizeof(UINT) * indices.size());
    sd.pSysMem   = indices.data();
    return SUCCEEDED(device->CreateBuffer(&bd, &sd, bufferIndices.GetAddressOf()));
}

void Malha::bind(ID3D11DeviceContext* contexto) const {
    UINT stride = sizeof(Vertice), offset = 0;
    contexto->IASetVertexBuffers(0, 1, bufferVertices.GetAddressOf(), &stride, &offset);
    contexto->IASetIndexBuffer(bufferIndices.Get(), DXGI_FORMAT_R32_UINT, 0);
}
