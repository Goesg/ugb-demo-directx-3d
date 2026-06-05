# Etapa 5 — Texturização

## Objetivo

Aplicar uma imagem PNG sobre as faces do cubo, introduzindo os conceitos de coordenadas UV, `ID3D11ShaderResourceView`, `ID3D11SamplerState` e `texture.Sample()` no pixel shader.

---

## O que foi implementado

### Arquivos criados

| Arquivo | Responsabilidade |
|---|---|
| `src/Textura.h` | Declaração do módulo de textura |
| `src/Textura.cpp` | Carregamento via stb_image e criação dos recursos D3D11 |

### Arquivos modificados

| Arquivo | O que mudou |
|---|---|
| `shaders/VertexShader.hlsl` | Cor removida, UV adicionada como `TEXCOORD0` |
| `shaders/PixelShader.hlsl` | `texture.Sample()` substitui a cor por vértice |
| `src/Renderizador.h` | `Vertice` atualizado para `posicao + uv`, `desenharCubo()` recebe `Textura&` |
| `src/Renderizador.cpp` | Input Layout atualizado, UVs no cubo, bind de SRV e Sampler |
| `src/Aplicacao.h` | Adicionado membro `Textura textura` |
| `src/Aplicacao.cpp` | `textura.carregar()` na inicialização |

---

## Dependência externa — stb_image

Antes de compilar, baixe o arquivo `stb_image.h`:

**Download:** https://raw.githubusercontent.com/nothings/stb/master/stb_image.h

Salve em: `external/stb/stb_image.h`

O `#define STB_IMAGE_IMPLEMENTATION` em `Textura.cpp` ativa a implementação. Esse define deve existir em **apenas um** arquivo `.cpp` do projeto.

---

## Conceitos Introduzidos

### Coordenadas UV

UV são coordenadas 2D normalizadas que mapeiam a textura sobre a superfície:

```
(0,0) ──── (1,0)
  │    textura  │
(0,1) ──── (1,1)
```

- `U` = eixo horizontal (0 = esquerda, 1 = direita)
- `V` = eixo vertical (0 = topo, 1 = base) no DirectX

**Atenção:** No DirectX, `V=0` é o topo da imagem. No OpenGL, `V=0` é a base. Texturas carregadas para DirectX não precisam ser invertidas verticalmente (ao contrário do OpenGL).

Cada vértice do cubo recebe coordenadas UV que cobrem a textura completa (0→1):

```cpp
{ XMFLOAT3(-0.5f,  0.5f, -0.5f), XMFLOAT2(0.0f, 0.0f) }, // canto superior esq
{ XMFLOAT3( 0.5f,  0.5f, -0.5f), XMFLOAT2(1.0f, 0.0f) }, // canto superior dir
{ XMFLOAT3( 0.5f, -0.5f, -0.5f), XMFLOAT2(1.0f, 1.0f) }, // canto inferior dir
{ XMFLOAT3(-0.5f, -0.5f, -0.5f), XMFLOAT2(0.0f, 1.0f) }, // canto inferior esq
```

---

### stb_image — Carregamento da Imagem

`stb_image` é uma biblioteca header-only que decodifica PNG, JPG e outros formatos:

```cpp
int largura, altura, canais;
unsigned char* pixels = stbi_load(caminho.c_str(), &largura, &altura, &canais, 4);
```

O `4` no último parâmetro força saída em **RGBA** (4 bytes por pixel), independente do formato original. Isso garante compatibilidade com `DXGI_FORMAT_R8G8B8A8_UNORM`.

Após o upload para a GPU, os pixels da CPU são liberados:

```cpp
stbi_image_free(pixels);
```

---

### ID3D11Texture2D — Textura na GPU

Cria o recurso de textura na memória da GPU:

```cpp
D3D11_TEXTURE2D_DESC desc = {};
desc.Width            = largura;
desc.Height           = altura;
desc.Format           = DXGI_FORMAT_R8G8B8A8_UNORM; // 1 byte por canal RGBA
desc.Usage            = D3D11_USAGE_DEFAULT;
desc.BindFlags        = D3D11_BIND_SHADER_RESOURCE;

D3D11_SUBRESOURCE_DATA sd = {};
sd.pSysMem     = pixels;
sd.SysMemPitch = largura * 4; // bytes por linha

device->CreateTexture2D(&desc, &sd, textura2D.GetAddressOf());
```

`SysMemPitch` é o stride de cada linha — quantos bytes até o início da próxima linha.

---

### ID3D11ShaderResourceView (SRV)

A `ID3D11Texture2D` não pode ser acessada diretamente pelo shader. É necessário criar uma "view" sobre ela:

```cpp
D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
srvDesc.Format        = DXGI_FORMAT_R8G8B8A8_UNORM;
srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
srvDesc.Texture2D.MipLevels = 1;

device->CreateShaderResourceView(textura2D.Get(), &srvDesc, srv.GetAddressOf());
```

O SRV é vinculado ao pixel shader no slot `t0`:

```cpp
contexto->PSSetShaderResources(0, 1, &srv);
```

---

### ID3D11SamplerState

O sampler define **como** a GPU lê pixels da textura — filtragem e comportamento nas bordas:

```cpp
D3D11_SAMPLER_DESC sd = {};
sd.Filter   = D3D11_FILTER_MIN_MAG_MIP_LINEAR; // filtragem bilinear (suave)
sd.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;       // repetir quando UV > 1
sd.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
```

**Modos de filtragem:**
| Filtro | Resultado |
|---|---|
| `POINT` | Pixel mais próximo (pixelado) |
| `LINEAR` | Interpolação bilinear (suave) |
| `ANISOTROPIC` | Alta qualidade em ângulos oblíquos |

**Modos de endereçamento:**
| Modo | Comportamento com UV fora de [0,1] |
|---|---|
| `WRAP` | Repete a textura |
| `CLAMP` | Estica o pixel da borda |
| `MIRROR` | Espelha a textura |

O sampler é vinculado ao pixel shader no slot `s0`:

```cpp
contexto->PSSetSamplers(0, 1, &smp);
```

---

### Pixel Shader com Textura

O shader usa `Texture2D.Sample()` para amostrar a cor no ponto UV interpolado:

```hlsl
Texture2D    textura    : register(t0);
SamplerState amostrador : register(s0);

float4 main(EntradaPS entrada) : SV_TARGET {
    return textura.Sample(amostrador, entrada.uv);
}
```

`register(t0)` e `register(s0)` correspondem aos slots `PSSetShaderResources(0,...)` e `PSSetSamplers(0,...)` no C++.

---

### Input Layout atualizado

O struct `Vertice` mudou — cor removida, UV adicionada. O Input Layout acompanha:

```cpp
D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0,                            ... },
    { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, ... },
};
```

`R32G32_FLOAT` = 2 floats = 8 bytes = `XMFLOAT2`.

---

## Preparação para testar

1. Baixe `stb_image.h` e salve em `external/stb/stb_image.h`
2. Coloque qualquer imagem PNG em `assets/textures/textura.png`
3. Compile e execute a partir da raiz do projeto

```powershell
msbuild Demo3D.vcxproj /p:Configuration=Debug /p:Platform=x64
.\build\x64\Debug\Demo3D.exe
```

---

## Diferenças DirectX vs OpenGL nesta etapa

| Aspecto | OpenGL | DirectX 11 |
|---|---|---|
| Criar textura | `glTexImage2D()` | `CreateTexture2D()` + `CreateShaderResourceView()` |
| Sampler | Integrado à textura (`glTexParameteri`) | Objeto separado (`ID3D11SamplerState`) |
| Bind textura | `glBindTexture()` | `PSSetShaderResources()` |
| Bind sampler | Automático | `PSSetSamplers()` |
| Origem V | V=0 na base (invertido vs imagem) | V=0 no topo (igual à imagem) |
| Sample no shader | `texture(sampler, uv)` (GLSL) | `textura.Sample(amostrador, uv)` (HLSL) |

---

## Resultado

O cubo exibe a imagem PNG mapeada em todas as 6 faces com filtragem bilinear. A câmera continua interativa com WASD e setas.

---

## Próxima etapa

**Etapa 6 — Iluminação Direcional**

Introduz:
- Normais no vertex buffer
- Constant buffer com dados de luz
- Cálculo de diffuse + ambient no pixel shader
