#include "Renderizador.h"
#include <string>

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
    scd.SampleDesc.Count                   = 1;
    scd.Windowed                           = TRUE;
    scd.SwapEffect                         = DXGI_SWAP_EFFECT_DISCARD;

    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    D3D_FEATURE_LEVEL niveisSuportados[] = { D3D_FEATURE_LEVEL_11_0 };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        flags, niveisSuportados, 1, D3D11_SDK_VERSION,
        &scd,
        swapChain.GetAddressOf(),
        device.GetAddressOf(),
        &featureLevel,
        contexto.GetAddressOf()
    );
    if (FAILED(hr)) return false;

    if (!criarRenderTarget())   return false;
    if (!compilarShaders())     return false;
    if (!criarBufferVertices()) return false;

    return true;
}

bool Renderizador::criarRenderTarget() {
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                      reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    if (FAILED(hr)) return false;

    hr = device->CreateRenderTargetView(backBuffer.Get(), nullptr,
                                        renderTargetView.GetAddressOf());
    if (FAILED(hr)) return false;

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

    contexto->OMSetRenderTargets(1, renderTargetView.GetAddressOf(),
                                 depthStencilView.Get());

    D3D11_VIEWPORT vp = {};
    vp.Width    = static_cast<float>(largura);
    vp.Height   = static_cast<float>(altura);
    vp.MinDepth = 0.0f;
    vp.MaxDepth = 1.0f;
    contexto->RSSetViewports(1, &vp);

    return true;
}

bool Renderizador::compilarShaders() {
    ComPtr<ID3DBlob> blobVS, blobPS, blobErro;

    // Compilar vertex shader a partir do arquivo .hlsl em runtime
    HRESULT hr = D3DCompileFromFile(
        L"shaders/VertexShader.hlsl", nullptr, nullptr,
        "main", "vs_5_0", D3DCOMPILE_DEBUG, 0,
        blobVS.GetAddressOf(), blobErro.GetAddressOf()
    );
    if (FAILED(hr)) {
        if (blobErro)
            OutputDebugStringA((char*)blobErro->GetBufferPointer());
        return false;
    }

    hr = D3DCompileFromFile(
        L"shaders/PixelShader.hlsl", nullptr, nullptr,
        "main", "ps_5_0", D3DCOMPILE_DEBUG, 0,
        blobPS.GetAddressOf(), blobErro.GetAddressOf()
    );
    if (FAILED(hr)) {
        if (blobErro)
            OutputDebugStringA((char*)blobErro->GetBufferPointer());
        return false;
    }

    hr = device->CreateVertexShader(blobVS->GetBufferPointer(),
                                    blobVS->GetBufferSize(),
                                    nullptr, vertexShader.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = device->CreatePixelShader(blobPS->GetBufferPointer(),
                                   blobPS->GetBufferSize(),
                                   nullptr, pixelShader.GetAddressOf());
    if (FAILED(hr)) return false;

    // Input Layout: descreve como os bytes do vertex buffer mapeiam para o shader
    // Deve corresponder exatamente ao struct Vertice e às semantics do VertexShader.hlsl
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = device->CreateInputLayout(layoutDesc, 2,
                                   blobVS->GetBufferPointer(),
                                   blobVS->GetBufferSize(),
                                   inputLayout.GetAddressOf());
    return SUCCEEDED(hr);
}

bool Renderizador::criarBufferVertices() {
    // Triângulo em NDC (Normalized Device Coordinates): X e Y entre -1 e +1
    // DirectX usa sistema Left-Handed: winding order clockwise para face frontal
    Vertice vertices[] = {
        { XMFLOAT3( 0.0f,  0.5f, 0.0f), XMFLOAT4(1.0f, 0.0f, 0.0f, 1.0f) }, // topo       — vermelho
        { XMFLOAT3( 0.5f, -0.5f, 0.0f), XMFLOAT4(0.0f, 1.0f, 0.0f, 1.0f) }, // direita    — verde
        { XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT4(0.0f, 0.0f, 1.0f, 1.0f) }, // esquerda   — azul
    };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage          = D3D11_USAGE_IMMUTABLE; // dados não mudam após criação
    bd.ByteWidth      = sizeof(vertices);
    bd.BindFlags      = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = vertices;

    return SUCCEEDED(device->CreateBuffer(&bd, &sd, bufferVertices.GetAddressOf()));
}

void Renderizador::limparTela(float r, float g, float b, float a) {
    float cor[4] = { r, g, b, a };
    contexto->ClearRenderTargetView(renderTargetView.Get(), cor);
    contexto->ClearDepthStencilView(depthStencilView.Get(),
                                    D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                    1.0f, 0);
}

void Renderizador::desenharTriangulo() {
    // Configurar o Input Assembler
    UINT stride = sizeof(Vertice);
    UINT offset = 0;
    contexto->IASetVertexBuffers(0, 1, bufferVertices.GetAddressOf(), &stride, &offset);
    contexto->IASetInputLayout(inputLayout.Get());
    contexto->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Vincular shaders ao pipeline
    contexto->VSSetShader(vertexShader.Get(), nullptr, 0);
    contexto->PSSetShader(pixelShader.Get(), nullptr, 0);

    // Desenhar 3 vértices (1 triângulo)
    contexto->Draw(3, 0);
}

void Renderizador::apresentar() {
    swapChain->Present(1, 0);
}
