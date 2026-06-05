#include "Renderizador.h"
#include <stdexcept>

bool Renderizador::inicializar(HWND hwnd, int largura, int altura) {
    this->largura = largura;
    this->altura  = altura;

    // Descrever o swap chain: double buffering, formato RGBA, janela alvo
    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount                        = 1;
    scd.BufferDesc.Width                   = largura;
    scd.BufferDesc.Height                  = altura;
    scd.BufferDesc.Format                  = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator   = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage                        = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow                       = hwnd;
    scd.SampleDesc.Count                   = 1; // sem MSAA por enquanto
    scd.Windowed                           = TRUE;
    scd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    D3D_FEATURE_LEVEL niveisSuportados[] = { D3D_FEATURE_LEVEL_11_0 };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr,                  // adaptador padrão
        D3D_DRIVER_TYPE_HARDWARE, // usar GPU
        nullptr,
        flags,
        niveisSuportados, 1,
        D3D11_SDK_VERSION,
        &scd,
        swapChain.GetAddressOf(),
        device.GetAddressOf(),
        &featureLevel,
        contexto.GetAddressOf()
    );

    if (FAILED(hr)) return false;

    return criarRenderTarget();
}

bool Renderizador::criarRenderTarget() {
    // Obter o back buffer do swap chain para criar o render target
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                      reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    if (FAILED(hr)) return false;

    hr = device->CreateRenderTargetView(backBuffer.Get(), nullptr,
                                        renderTargetView.GetAddressOf());
    if (FAILED(hr)) return false;

    // Criar depth/stencil buffer para teste de profundidade correto entre faces
    D3D11_TEXTURE2D_DESC dsd = {};
    dsd.Width            = largura;
    dsd.Height           = altura;
    dsd.MipLevels        = 1;
    dsd.ArraySize        = 1;
    dsd.Format           = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsd.SampleDesc.Count = 1;
    dsd.Usage            = D3D11_USAGE_DEFAULT;
    dsd.BindFlags        = D3D11_BIND_DEPTH_STENCIL;

    hr = device->CreateTexture2D(&dsd, nullptr, depthStencilBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr,
                                        depthStencilView.GetAddressOf());
    if (FAILED(hr)) return false;

    // Vincular render target e depth buffer ao Output Merger
    contexto->OMSetRenderTargets(1, renderTargetView.GetAddressOf(),
                                 depthStencilView.Get());

    // Configurar viewport para cobrir a janela inteira
    D3D11_VIEWPORT vp = {};
    vp.Width    = static_cast<float>(largura);
    vp.Height   = static_cast<float>(altura);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    contexto->RSSetViewports(1, &vp);

    return true;
}

void Renderizador::limparTela(float r, float g, float b, float a) {
    float cor[4] = { r, g, b, a };
    contexto->ClearRenderTargetView(renderTargetView.Get(), cor);
    contexto->ClearDepthStencilView(depthStencilView.Get(),
                                    D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                    1.0f, 0);
}

void Renderizador::apresentar() {
    swapChain->Present(1, 0); // vsync ativo (1)
}
