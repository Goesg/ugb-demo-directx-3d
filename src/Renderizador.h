#pragma once
#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <wrl/client.h>
#include "Textura.h"

using Microsoft::WRL::ComPtr;
using namespace DirectX;

// Vértice com posição 3D e coordenadas de textura UV
struct Vertice {
    XMFLOAT3 posicao;
    XMFLOAT2 uv;
};

struct __declspec(align(16)) DadosConstantes {
    XMMATRIX matrizMundo;
    XMMATRIX matrizVisao;
    XMMATRIX matrizProjecao;
};

class Renderizador {
public:
    Renderizador() = default;
    ~Renderizador() = default;

    bool inicializar(HWND hwnd, int largura, int altura);
    void limparTela(float r, float g, float b, float a = 1.0f);
    void desenharCubo(const XMMATRIX& mundo, const XMMATRIX& visao,
                      const XMMATRIX& projecao, Textura& textura);
    void apresentar();

    ID3D11Device*        obterDevice()   const { return device.Get(); }
    ID3D11DeviceContext* obterContexto() const { return contexto.Get(); }

private:
    bool criarRenderTarget();
    bool compilarShaders();
    bool criarGeometriaCubo();
    bool criarConstantBuffer();

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
    ComPtr<ID3D11Buffer>            bufferIndices;
    ComPtr<ID3D11Buffer>            bufferConstante;

    int largura = 0;
    int altura  = 0;
};
