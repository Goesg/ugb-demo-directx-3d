#include "Renderizador.h"

bool Renderizador::inicializar(HWND hwnd, int largura, int altura) {
    this->largura = largura;
    this->altura  = altura;

    DXGI_SWAP_CHAIN_DESC scd = {};
    scd.BufferCount = 1;
    scd.BufferDesc.Width   = largura;
    scd.BufferDesc.Height  = altura;
    scd.BufferDesc.Format  = DXGI_FORMAT_R8G8B8A8_UNORM;
    scd.BufferDesc.RefreshRate.Numerator   = 60;
    scd.BufferDesc.RefreshRate.Denominator = 1;
    scd.BufferUsage  = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    scd.OutputWindow = hwnd;
    scd.SampleDesc.Count = 1;
    scd.Windowed   = TRUE;
    scd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

    UINT flags = 0;
#ifdef _DEBUG
    flags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    D3D_FEATURE_LEVEL niveisSuportados[] = { D3D_FEATURE_LEVEL_11_0 };

    HRESULT hr = D3D11CreateDeviceAndSwapChain(
        nullptr, D3D_DRIVER_TYPE_HARDWARE, nullptr,
        flags, niveisSuportados, 1, D3D11_SDK_VERSION,
        &scd, swapChain.GetAddressOf(), device.GetAddressOf(),
        &featureLevel, contexto.GetAddressOf()
    );
    if (FAILED(hr)) return false;

    if (!criarRenderTarget())    return false;
    if (!compilarShaders())      return false;
    if (!criarGeometriaCubo())   return false;
    if (!criarConstantBuffers()) return false;

    return true;
}

bool Renderizador::criarRenderTarget() {
    ComPtr<ID3D11Texture2D> backBuffer;
    HRESULT hr = swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D),
                                      reinterpret_cast<void**>(backBuffer.GetAddressOf()));
    if (FAILED(hr)) return false;

    hr = device->CreateRenderTargetView(backBuffer.Get(), nullptr, renderTargetView.GetAddressOf());
    if (FAILED(hr)) return false;

    D3D11_TEXTURE2D_DESC dsd = {};
    dsd.Width = largura; dsd.Height = altura;
    dsd.MipLevels = 1; dsd.ArraySize = 1;
    dsd.Format           = DXGI_FORMAT_D24_UNORM_S8_UINT;
    dsd.SampleDesc.Count = 1;
    dsd.Usage     = D3D11_USAGE_DEFAULT;
    dsd.BindFlags = D3D11_BIND_DEPTH_STENCIL;

    hr = device->CreateTexture2D(&dsd, nullptr, depthStencilBuffer.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = device->CreateDepthStencilView(depthStencilBuffer.Get(), nullptr, depthStencilView.GetAddressOf());
    if (FAILED(hr)) return false;

    contexto->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

    D3D11_VIEWPORT vp = {};
    vp.Width = static_cast<float>(largura);
    vp.Height = static_cast<float>(altura);
    vp.MinDepth = 0.0f; vp.MaxDepth = 1.0f;
    contexto->RSSetViewports(1, &vp);
    return true;
}

bool Renderizador::compilarShaders() {
    ComPtr<ID3DBlob> blobVS, blobPS, blobErro;

    HRESULT hr = D3DCompileFromFile(L"shaders/VertexShader.hlsl", nullptr, nullptr,
        "main", "vs_5_0", D3DCOMPILE_DEBUG, 0, blobVS.GetAddressOf(), blobErro.GetAddressOf());
    if (FAILED(hr)) {
        if (blobErro) OutputDebugStringA((char*)blobErro->GetBufferPointer());
        return false;
    }

    hr = D3DCompileFromFile(L"shaders/PixelShader.hlsl", nullptr, nullptr,
        "main", "ps_5_0", D3DCOMPILE_DEBUG, 0, blobPS.GetAddressOf(), blobErro.GetAddressOf());
    if (FAILED(hr)) {
        if (blobErro) OutputDebugStringA((char*)blobErro->GetBufferPointer());
        return false;
    }

    hr = device->CreateVertexShader(blobVS->GetBufferPointer(), blobVS->GetBufferSize(),
                                    nullptr, vertexShader.GetAddressOf());
    if (FAILED(hr)) return false;

    hr = device->CreatePixelShader(blobPS->GetBufferPointer(), blobPS->GetBufferSize(),
                                   nullptr, pixelShader.GetAddressOf());
    if (FAILED(hr)) return false;

    // Input Layout: POSITION (float3) + NORMAL (float3) + TEXCOORD (float2)
    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",   0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = device->CreateInputLayout(layoutDesc, 3,
                                   blobVS->GetBufferPointer(), blobVS->GetBufferSize(),
                                   inputLayout.GetAddressOf());
    return SUCCEEDED(hr);
}

bool Renderizador::criarGeometriaCubo() {
    // Cada face tem sua normal apontando para fora do cubo
    Vertice vertices[] = {
        // frente — normal (0, 0, -1)
        { XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0,0,-1), XMFLOAT2(0,0) },
        { XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3(0,0,-1), XMFLOAT2(1,0) },
        { XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3(0,0,-1), XMFLOAT2(1,1) },
        { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0,0,-1), XMFLOAT2(0,1) },
        // trás — normal (0, 0, 1)
        { XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3(0,0,1), XMFLOAT2(0,0) },
        { XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0,0,1), XMFLOAT2(1,0) },
        { XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0,0,1), XMFLOAT2(1,1) },
        { XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3(0,0,1), XMFLOAT2(0,1) },
        // cima — normal (0, 1, 0)
        { XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(0,1,0), XMFLOAT2(0,0) },
        { XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3(0,1,0), XMFLOAT2(1,0) },
        { XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3(0,1,0), XMFLOAT2(1,1) },
        { XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(0,1,0), XMFLOAT2(0,1) },
        // baixo — normal (0, -1, 0)
        { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(0,-1,0), XMFLOAT2(0,0) },
        { XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3(0,-1,0), XMFLOAT2(1,0) },
        { XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3(0,-1,0), XMFLOAT2(1,1) },
        { XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(0,-1,0), XMFLOAT2(0,1) },
        // direita — normal (1, 0, 0)
        { XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT3(1,0,0), XMFLOAT2(0,0) },
        { XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT3(1,0,0), XMFLOAT2(1,0) },
        { XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT3(1,0,0), XMFLOAT2(1,1) },
        { XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT3(1,0,0), XMFLOAT2(0,1) },
        // esquerda — normal (-1, 0, 0)
        { XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT3(-1,0,0), XMFLOAT2(0,0) },
        { XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT3(-1,0,0), XMFLOAT2(1,0) },
        { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT3(-1,0,0), XMFLOAT2(1,1) },
        { XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT3(-1,0,0), XMFLOAT2(0,1) },
    };

    UINT indices[] = {
         0,  1,  2,   0,  2,  3,
         4,  5,  6,   4,  6,  7,
         8,  9, 10,   8, 10, 11,
        13, 12, 15,  13, 15, 14,
        16, 17, 18,  16, 18, 19,
        20, 21, 22,  20, 22, 23,
    };

    D3D11_BUFFER_DESC bd = {};
    bd.Usage = D3D11_USAGE_IMMUTABLE; bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    bd.ByteWidth = sizeof(vertices);
    D3D11_SUBRESOURCE_DATA sd = {}; sd.pSysMem = vertices;
    HRESULT hr = device->CreateBuffer(&bd, &sd, bufferVertices.GetAddressOf());
    if (FAILED(hr)) return false;

    bd.ByteWidth = sizeof(indices); bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    sd.pSysMem = indices;
    return SUCCEEDED(device->CreateBuffer(&bd, &sd, bufferIndices.GetAddressOf()));
}

bool Renderizador::criarConstantBuffers() {
    D3D11_BUFFER_DESC bd = {};
    bd.Usage          = D3D11_USAGE_DYNAMIC;
    bd.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    bd.ByteWidth = sizeof(DadosConstantes);
    HRESULT hr = device->CreateBuffer(&bd, nullptr, bufferConstante.GetAddressOf());
    if (FAILED(hr)) return false;

    bd.ByteWidth = sizeof(DadosLuz);
    return SUCCEEDED(device->CreateBuffer(&bd, nullptr, bufferLuz.GetAddressOf()));
}

void Renderizador::limparTela(float r, float g, float b, float a) {
    float cor[4] = { r, g, b, a };
    contexto->ClearRenderTargetView(renderTargetView.Get(), cor);
    contexto->ClearDepthStencilView(depthStencilView.Get(),
                                    D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderizador::desenharCubo(const XMMATRIX& mundo, const XMMATRIX& visao,
                                 const XMMATRIX& projecao, Textura& textura,
                                 const DadosLuz& luz) {
    // Atualizar constant buffer de transformação (slot b0)
    D3D11_MAPPED_SUBRESOURCE msr;
    contexto->Map(bufferConstante.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    DadosConstantes* dc = reinterpret_cast<DadosConstantes*>(msr.pData);
    dc->matrizMundo    = XMMatrixTranspose(mundo);
    dc->matrizVisao    = XMMatrixTranspose(visao);
    dc->matrizProjecao = XMMatrixTranspose(projecao);
    contexto->Unmap(bufferConstante.Get(), 0);
    contexto->VSSetConstantBuffers(0, 1, bufferConstante.GetAddressOf());

    // Atualizar constant buffer de luz (slot b1)
    contexto->Map(bufferLuz.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    *reinterpret_cast<DadosLuz*>(msr.pData) = luz;
    contexto->Unmap(bufferLuz.Get(), 0);
    contexto->PSSetConstantBuffers(1, 1, bufferLuz.GetAddressOf());

    // Textura e sampler
    ID3D11ShaderResourceView* srv = textura.obterSRV();
    ID3D11SamplerState*       smp = textura.obterAmostrador();
    contexto->PSSetShaderResources(0, 1, &srv);
    contexto->PSSetSamplers(0, 1, &smp);

    UINT stride = sizeof(Vertice), offset = 0;
    contexto->IASetVertexBuffers(0, 1, bufferVertices.GetAddressOf(), &stride, &offset);
    contexto->IASetIndexBuffer(bufferIndices.Get(), DXGI_FORMAT_R32_UINT, 0);
    contexto->IASetInputLayout(inputLayout.Get());
    contexto->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    contexto->VSSetShader(vertexShader.Get(), nullptr, 0);
    contexto->PSSetShader(pixelShader.Get(), nullptr, 0);

    contexto->DrawIndexed(36, 0, 0);
}

void Renderizador::apresentar() {
    swapChain->Present(1, 0);
}
