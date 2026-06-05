# Etapa 2 — Triângulo Colorido

## Objetivo

Renderizar o primeiro triângulo na tela com cores por vértice, introduzindo os conceitos fundamentais do pipeline gráfico do DirectX 11: vertex buffer, shaders HLSL, input layout e draw call.

---

## O que foi implementado

### Arquivos criados

| Arquivo | Responsabilidade |
|---|---|
| `shaders/VertexShader.hlsl` | Processa cada vértice — posição e cor |
| `shaders/PixelShader.hlsl` | Define a cor final de cada pixel |

### Arquivos modificados

| Arquivo | O que mudou |
|---|---|
| `src/Renderizador.h` | Adicionado struct `Vertice`, novos objetos D3D11 e método `desenharTriangulo()` |
| `src/Renderizador.cpp` | Implementação de `compilarShaders()`, `criarBufferVertices()` e `desenharTriangulo()` |
| `src/Aplicacao.cpp` | Chamada de `desenharTriangulo()` no loop de renderização |
| `Demo3D.vcxproj` | Shaders HLSL adicionados ao projeto |

---

## Conceitos Introduzidos

### O Pipeline Gráfico do DirectX 11

O pipeline é a sequência de etapas que a GPU executa para transformar vértices em pixels na tela. Nesta etapa, duas delas entram em ação:

```
CPU envia vértices
      ↓
[IA] Input Assembler    → lê os vértices do buffer e monta triângulos
      ↓
[VS] Vertex Shader      → processa cada vértice individualmente (HLSL)
      ↓
[RS] Rasterizador       → converte triângulos em fragmentos de pixels
                          e interpola os valores entre os vértices
      ↓
[PS] Pixel Shader       → define a cor final de cada pixel (HLSL)
      ↓
[OM] Output Merger      → escreve o pixel no render target (tela)
```

---

### Struct Vertice

Define o formato de cada vértice em C++. Cada vértice carrega sua posição 3D e uma cor RGBA:

```cpp
struct Vertice {
    XMFLOAT3 posicao; // X, Y, Z  (3 floats = 12 bytes)
    XMFLOAT4 cor;     // R, G, B, A (4 floats = 16 bytes)
};
// Tamanho total por vértice: 28 bytes
```

**Importante:** este struct deve corresponder exatamente ao que o Input Layout declara e ao que o Vertex Shader espera receber.

---

### Vertex Buffer

O vertex buffer é um bloco de memória na GPU que armazena os vértices do triângulo.

```cpp
Vertice vertices[] = {
    { XMFLOAT3( 0.0f,  0.5f, 0.0f), XMFLOAT4(1, 0, 0, 1) }, // topo     — vermelho
    { XMFLOAT3( 0.5f, -0.5f, 0.0f), XMFLOAT4(0, 1, 0, 1) }, // direita  — verde
    { XMFLOAT3(-0.5f, -0.5f, 0.0f), XMFLOAT4(0, 0, 1, 1) }, // esquerda — azul
};
```

As coordenadas estão em **NDC (Normalized Device Coordinates)**:
- Centro da tela = `(0, 0)`
- Borda direita = `X = +1`, borda esquerda = `X = -1`
- Borda superior = `Y = +1`, borda inferior = `Y = -1`

O buffer é criado com `D3D11_USAGE_IMMUTABLE` porque os dados não mudam após o upload para a GPU:

```cpp
D3D11_BUFFER_DESC bd = {};
bd.Usage     = D3D11_USAGE_IMMUTABLE;
bd.ByteWidth = sizeof(vertices);
bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;

D3D11_SUBRESOURCE_DATA sd = {};
sd.pSysMem = vertices; // ponteiro para os dados na CPU

device->CreateBuffer(&bd, &sd, bufferVertices.GetAddressOf());
```

---

### Input Layout

O Input Layout é uma descrição de como os bytes do vertex buffer se mapeiam para as variáveis do Vertex Shader. É a "ponte" entre o struct C++ e o HLSL.

```cpp
D3D11_INPUT_ELEMENT_DESC layoutDesc[] = {
    // Nome semantic, índice, formato,        slot, offset,                      instância
    { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, 0,                            D3D11_INPUT_PER_VERTEX_DATA, 0 },
    { "COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};
```

**Leitura:** o primeiro campo `POSITION` começa no byte 0 e ocupa 12 bytes (`R32G32B32` = 3 floats). O segundo campo `COLOR` começa logo depois (`APPEND_ALIGNED`) e ocupa 16 bytes (`R32G32B32A32` = 4 floats).

O Input Layout é validado contra o bytecode do Vertex Shader no momento da criação — se houver divergência entre o C++ e o HLSL, a criação falha.

---

### Shaders HLSL

#### Vertex Shader (`VertexShader.hlsl`)

Executado **uma vez por vértice**. Recebe os dados do vertex buffer e retorna a posição final em clip space:

```hlsl
struct EntradaVS {
    float3 posicao : POSITION; // semantic "POSITION" vincula ao Input Layout
    float4 cor     : COLOR;
};

struct SaidaVS {
    float4 posicao : SV_POSITION; // SV_ = System Value, interpretado pela GPU
    float4 cor     : COLOR;
};

SaidaVS main(EntradaVS entrada) {
    SaidaVS saida;
    saida.posicao = float4(entrada.posicao, 1.0f); // w=1 para ponto (não vetor)
    saida.cor     = entrada.cor;
    return saida;
}
```

**Semantics** são rótulos que indicam para que serve cada variável:
- `POSITION` → posição do vértice (vinda do vertex buffer)
- `SV_POSITION` → posição final em clip space (interpretada pela GPU)
- `COLOR` → dado personalizado passado ao pixel shader

Na Etapa 2 não há transformação de coordenadas — os vértices já estão em NDC.

#### Pixel Shader (`PixelShader.hlsl`)

Executado **uma vez por pixel** coberto pelo triângulo. Recebe valores interpolados entre os vértices:

```hlsl
float4 main(EntradaPS entrada) : SV_TARGET {
    return entrada.cor; // devolve a cor interpolada
}
```

`SV_TARGET` indica que o valor retornado é a cor final do pixel escrita no render target.

#### Interpolação automática do Rasterizador

O rasterizador interpola automaticamente os valores de saída do Vertex Shader entre os vértices. Por isso o triângulo exibe um gradiente RGB — cada pixel recebe uma combinação ponderada das três cores conforme sua posição no triângulo.

---

### Compilação de Shaders em Runtime

Os shaders são compilados a partir do arquivo `.hlsl` quando a aplicação inicia, usando `D3DCompileFromFile`:

```cpp
D3DCompileFromFile(
    L"shaders/VertexShader.hlsl", // caminho do arquivo
    nullptr,                       // sem defines
    nullptr,                       // sem includes customizados
    "main",                        // nome da função entry point
    "vs_5_0",                      // shader model: vertex shader 5.0
    D3DCOMPILE_DEBUG,              // flags de compilação
    0,
    blobVS.GetAddressOf(),         // bytecode compilado (saída)
    blobErro.GetAddressOf()        // mensagem de erro (saída)
);
```

**Atenção:** erros HLSL aparecem apenas em runtime, não durante o build do C++. Verificar sempre o `blobErro` e o Output do Visual Studio quando a tela aparecer preta.

O caminho `"shaders/VertexShader.hlsl"` é relativo ao **diretório de trabalho** do executável. Por isso o `.exe` deve ser rodado a partir da raiz do projeto.

---

### Draw Call

Com tudo configurado no Input Assembler, o draw call instrui a GPU a processar os vértices:

```cpp
contexto->IASetVertexBuffers(0, 1, bufferVertices.GetAddressOf(), &stride, &offset);
contexto->IASetInputLayout(inputLayout.Get());
contexto->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
contexto->VSSetShader(vertexShader.Get(), nullptr, 0);
contexto->PSSetShader(pixelShader.Get(), nullptr, 0);
contexto->Draw(3, 0); // 3 vértices, começando no índice 0
```

`TRIANGLELIST` significa que cada grupo de 3 vértices forma um triângulo independente. Com 3 vértices, temos exatamente 1 triângulo.

---

## Winding Order — Por que a ordem dos vértices importa

O DirectX usa **clockwise (sentido horário)** como face frontal por padrão. O backface culling descarta automaticamente faces com winding counter-clockwise, pois estão "de costas" para a câmera.

```
Nosso triângulo (visto de frente):

        topo (0, 0.5)
           *
          / \
         /   \
        /     \
       *-------*
  esq(-0.5)  dir(0.5)

Ordem: topo → direita → esquerda = clockwise ✓
```

Se os vértices fossem declarados em ordem counter-clockwise, o triângulo ficaria invisível.

---

## Diferenças DirectX vs OpenGL nesta etapa

| Aspecto | OpenGL | DirectX 11 |
|---|---|---|
| Face frontal | Counter-clockwise | Clockwise |
| Compilar shader | Em runtime com `glShaderSource` | Em runtime com `D3DCompileFromFile` |
| Vincular shader | `glUseProgram()` | `VSSetShader()` + `PSSetShader()` |
| Vertex buffer | `glGenBuffers` + `glBindBuffer` | `CreateBuffer` + `IASetVertexBuffers` |
| Input Layout | Implícito via `glVertexAttribPointer` | Explícito via `CreateInputLayout` |
| Draw call | `glDrawArrays(GL_TRIANGLES, 0, 3)` | `Draw(3, 0)` |

---

## Resultado

Triângulo com gradiente de cores RGB (vermelho no topo, verde à direita, azul à esquerda) sobre fundo azul escuro. As cores são interpoladas automaticamente pelo rasterizador.

---

## Próxima etapa

**Etapa 3 — Cubo 3D**

Introduz:
- Index Buffer (reutilizar vértices)
- Constant Buffer com matrizes World / View / Projection
- Perspectiva 3D com `XMMatrixPerspectiveFovLH`
- Rotação ao longo do tempo com delta time
