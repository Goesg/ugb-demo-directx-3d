#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <directxmath.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;
using namespace DirectX;

// Representa um vértice com posição 3D e cor RGBA
struct Vertice {
    XMFLOAT3 posicao;
    XMFLOAT4 cor;
};

// Gerencia o dispositivo D3D11, swap chain e pipeline de renderização
class Renderizador {
public:
    Renderizador() = default;
    ~Renderizador() = default;

    bool inicializar(HWND hwnd, int largura, int altura);
    void limparTela(float r, float g, float b, float a = 1.0f);
    void desenharTriangulo();
    void apresentar();

    ID3D11Device*        obterDevice()   const { return device.Get(); }
    ID3D11DeviceContext* obterContexto() const { return contexto.Get(); }

private:
    bool criarRenderTarget();
    bool compilarShaders();
    bool criarBufferVertices();

    ComPtr<ID3D11Device>            device;
    ComPtr<ID3D11DeviceContext>     contexto;
    ComPtr<IDXGISwapChain>          swapChain;
    ComPtr<ID3D11RenderTargetView>  renderTargetView;
    ComPtr<ID3D11DepthStencilView>  depthStencilView;
    ComPtr<ID3D11Texture2D>         depthStencilBuffer;

    ComPtr<ID3D11VertexShader>      vertexShader;
    ComPtr<ID3D11PixelShader>       pixelShader;
    ComPtr<ID3D11InputLayout>       inputLayout;
    ComPtr<ID3D11Buffer>            bufferVertices;

    int largura = 0;
    int altura  = 0;
};
