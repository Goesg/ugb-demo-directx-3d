#include "Textura.h"

// Ativar implementação do stb_image (apenas uma vez em todo o projeto)
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

bool Textura::carregar(ID3D11Device* device, ID3D11DeviceContext* contexto,
                       const std::string& caminho) {
    // Carregar imagem do disco — forçar 4 canais RGBA
    int largura, altura, canais;
    unsigned char* pixels = stbi_load(caminho.c_str(), &largura, &altura, &canais, 4);
    if (!pixels) return false;

    // Criar textura 2D na GPU
    D3D11_TEXTURE2D_DESC desc = {};
    desc.Width            = largura;
    desc.Height           = altura;
    desc.MipLevels        = 1;
    desc.ArraySize        = 1;
    desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM;
    desc.SampleDesc.Count = 1;
    desc.Usage            = D3D11_USAGE_DEFAULT;
    desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

    D3D11_SUBRESOURCE_DATA sd = {};
    sd.pSysMem     = pixels;
    sd.SysMemPitch = largura * 4; // bytes por linha (4 canais × 1 byte cada)

    ComPtr<ID3D11Texture2D> textura2D;
    HRESULT hr = device->CreateTexture2D(&desc, &sd, textura2D.GetAddressOf());
    stbi_image_free(pixels); // liberar memória da CPU após upload para a GPU
    if (FAILED(hr)) return false;

    // Criar Shader Resource View — permite que o shader acesse a textura
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format                    = desc.Format;
    srvDesc.ViewDimension             = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels       = 1;
    srvDesc.Texture2D.MostDetailedMip = 0;

    hr = device->CreateShaderResourceView(textura2D.Get(), &srvDesc, srv.GetAddressOf());
    if (FAILED(hr)) return false;

    // Criar Sampler State — define como a GPU filtra pixels da textura
    D3D11_SAMPLER_DESC sd2 = {};
    sd2.Filter         = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // filtragem bilinear
    sd2.AddressU       = D3D11_TEXTURE_ADDRESS_WRAP;       // repetir textura em U
    sd2.AddressV       = D3D11_TEXTURE_ADDRESS_WRAP;       // repetir textura em V
    sd2.AddressW       = D3D11_TEXTURE_ADDRESS_WRAP;
    sd2.ComparisonFunc = D3D11_COMPARISON_NEVER;
    sd2.MinLOD         = 0;
    sd2.MaxLOD         = D3D11_FLOAT32_MAX;

    return SUCCEEDED(device->CreateSamplerState(&sd2, amostrador.GetAddressOf()));
}
