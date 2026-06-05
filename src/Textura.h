#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <string>

using Microsoft::WRL::ComPtr;

// Carrega uma imagem PNG e cria os recursos D3D11 necessários para texturização
class Textura {
public:
    bool carregar(ID3D11Device* device, ID3D11DeviceContext* contexto,
                  const std::string& caminho);

    ID3D11ShaderResourceView* obterSRV()      const { return srv.Get(); }
    ID3D11SamplerState*       obterAmostrador() const { return amostrador.Get(); }

private:
    ComPtr<ID3D11ShaderResourceView> srv;
    ComPtr<ID3D11SamplerState>       amostrador;
};
