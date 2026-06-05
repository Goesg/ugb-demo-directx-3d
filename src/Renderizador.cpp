#include "Renderizador.h"

bool Renderizador::inicializar(HWND hwnd, int largura, int altura) {
    this->largura = largura;
    this->altura  = altura;

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

    if (!criarRenderTarget())    return false;
    if (!compilarShaders())      return false;
    if (!criarGeometriaCubo())   return false;
    if (!criarConstantBuffer())  return false;

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

    contexto->OMSetRenderTargets(1, renderTargetView.GetAddressOf(), depthStencilView.Get());

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

    HRESULT hr = D3DCompileFromFile(
        L"shaders/VertexShader.hlsl", nullptr, nullptr,
        "main", "vs_5_0", D3DCOMPILE_DEBUG, 0,
        blobVS.GetAddressOf(), blobErro.GetAddressOf()
    );
    if (FAILED(hr)) {
        if (blobErro) OutputDebugStringA((char*)blobErro->GetBufferPointer());
        return false;
    }

    hr = D3DCompileFromFile(
        L"shaders/PixelShader.hlsl", nullptr, nullptr,
        "main", "ps_5_0", D3DCOMPILE_DEBUG, 0,
        blobPS.GetAddressOf(), blobErro.GetAddressOf()
    );
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

    D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    hr = device->CreateInputLayout(layoutDesc, 2,
                                   blobVS->GetBufferPointer(), blobVS->GetBufferSize(),
                                   inputLayout.GetAddressOf());
    return SUCCEEDED(hr);
}

bool Renderizador::criarGeometriaCubo() {
    // 8 vértices do cubo — cada face tem uma cor diferente para facilitar visualização
    Vertice vertices[] = {
        // frente (vermelho)
        { XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f) },
        { XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f) },
        { XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f) },
        { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(1.0f, 0.2f, 0.2f, 1.0f) },
        // trás (verde)
        { XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f) },
        { XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f) },
        { XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f) },
        { XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT4(0.2f, 1.0f, 0.2f, 1.0f) },
        // cima (azul)
        { XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT4(0.2f, 0.4f, 1.0f, 1.0f) },
        { XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT4(0.2f, 0.4f, 1.0f, 1.0f) },
        { XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT4(0.2f, 0.4f, 1.0f, 1.0f) },
        { XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT4(0.2f, 0.4f, 1.0f, 1.0f) },
        // baixo (amarelo)
        { XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT4(1.0f, 1.0f, 0.2f, 1.0f) },
        { XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT4(1.0f, 1.0f, 0.2f, 1.0f) },
        { XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT4(1.0f, 1.0f, 0.2f, 1.0f) },
        { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(1.0f, 1.0f, 0.2f, 1.0f) },
        // direita (magenta)
        { XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT4(1.0f, 0.2f, 1.0f, 1.0f) },
        { XMFLOAT3( 0.5f,  0.5f,  0.5f), XMFLOAT4(1.0f, 0.2f, 1.0f, 1.0f) },
        { XMFLOAT3( 0.5f, -0.5f,  0.5f), XMFLOAT4(1.0f, 0.2f, 1.0f, 1.0f) },
        { XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT4(1.0f, 0.2f, 1.0f, 1.0f) },
        // esquerda (ciano)
        { XMFLOAT3(-0.5f,  0.5f,  0.5f), XMFLOAT4(0.2f, 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT4(0.2f, 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT4(0.2f, 1.0f, 1.0f, 1.0f) },
        { XMFLOAT3(-0.5f, -0.5f,  0.5f), XMFLOAT4(0.2f, 1.0f, 1.0f, 1.0f) },
    };

    // 6 faces × 2 triângulos × 3 índices = 36 índices
    // Cada face usa 4 vértices (quad), divididos em 2 triângulos clockwise
    UINT indices[] = {
         0,  1,  2,   0,  2,  3,  // frente
         5,  4,  7,   5,  7,  6,  // trás
         8,  9, 10,   8, 10, 11,  // cima
        13, 12, 15,  13, 15, 14,  // baixo
        16, 17, 18,  16, 18, 19,  // direita
        20, 21, 22,  20, 22, 23,  // esquerda
    };

    // Criar vertex buffer
    D3D11_BUFFER_DESC bd = {};
    bd.Usage     = D3D11_USAGE_IMMUTABLE;
    bd.ByteWidth = sizeof(vertices);
    bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem = vertices;
    HRESULT hr = device->CreateBuffer(&bd, &sd, bufferVertices.GetAddressOf());
    if (FAILED(hr)) return false;

    // Criar index buffer
    bd.ByteWidth = sizeof(indices);
    bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
    sd.pSysMem   = indices;
    hr = device->CreateBuffer(&bd, &sd, bufferIndices.GetAddressOf());
    return SUCCEEDED(hr);
}

bool Renderizador::criarConstantBuffer() {
    // DYNAMIC + CPU_ACCESS_WRITE permite atualizar as matrizes a cada frame via Map/Unmap
    D3D11_BUFFER_DESC bd = {};
    bd.Usage          = D3D11_USAGE_DYNAMIC;
    bd.ByteWidth      = sizeof(DadosConstantes);
    bd.BindFlags      = D3D11_BIND_CONSTANT_BUFFER;
    bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;

    return SUCCEEDED(device->CreateBuffer(&bd, nullptr, bufferConstante.GetAddressOf()));
}

void Renderizador::limparTela(float r, float g, float b, float a) {
    float cor[4] = { r, g, b, a };
    contexto->ClearRenderTargetView(renderTargetView.Get(), cor);
    contexto->ClearDepthStencilView(depthStencilView.Get(),
                                    D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL, 1.0f, 0);
}

void Renderizador::desenharCubo(const XMMATRIX& mundo, const XMMATRIX& visao, const XMMATRIX& projecao) {
    // Atualizar constant buffer com as matrizes do frame atual
    // HLSL espera column-major → transpor antes de enviar
    D3D11_MAPPED_SUBRESOURCE msr;
    contexto->Map(bufferConstante.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);
    DadosConstantes* dados = reinterpret_cast<DadosConstantes*>(msr.pData);
    dados->matrizMundo    = XMMatrixTranspose(mundo);
    dados->matrizVisao    = XMMatrixTranspose(visao);
    dados->matrizProjecao = XMMatrixTranspose(projecao);
    contexto->Unmap(bufferConstante.Get(), 0);

    // Vincular constant buffer ao vertex shader no slot b0
    contexto->VSSetConstantBuffers(0, 1, bufferConstante.GetAddressOf());

    // Configurar Input Assembler
    UINT stride = sizeof(Vertice);
    UINT offset = 0;
    contexto->IASetVertexBuffers(0, 1, bufferVertices.GetAddressOf(), &stride, &offset);
    contexto->IASetIndexBuffer(bufferIndices.Get(), DXGI_FORMAT_R32_UINT, 0);
    contexto->IASetInputLayout(inputLayout.Get());
    contexto->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

    // Vincular shaders
    contexto->VSSetShader(vertexShader.Get(), nullptr, 0);
    contexto->PSSetShader(pixelShader.Get(), nullptr, 0);

    // Desenhar 36 índices (6 faces × 2 triângulos × 3 vértices)
    contexto->DrawIndexed(36, 0, 0);
}

void Renderizador::apresentar() {
    swapChain->Present(1, 0);
}
