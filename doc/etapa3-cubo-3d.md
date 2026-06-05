# Etapa 3 — Cubo 3D com Transformações

## Objetivo

Renderizar um cubo 3D com 6 faces coloridas rotacionando continuamente, introduzindo os conceitos de index buffer, matrizes de transformação (World/View/Projection) e delta time.

---

## O que foi implementado

### Arquivos modificados

| Arquivo | O que mudou |
|---|---|
| `shaders/VertexShader.hlsl` | Adicionado `cbuffer` com matrizes MVP e multiplicação no shader |
| `src/Renderizador.h` | Adicionado `DadosConstantes`, `bufferIndices`, `bufferConstante` e novo método `desenharCubo()` |
| `src/Renderizador.cpp` | Implementação de `criarGeometriaCubo()`, `criarConstantBuffer()` e `desenharCubo()` |
| `src/Aplicacao.h` | Adicionadas matrizes MVP, `anguloRotacao` e delta time |
| `src/Aplicacao.cpp` | Câmera fixa, projeção perspectiva, rotação por delta time |

---

## Conceitos Introduzidos

### Index Buffer

Na Etapa 2 cada triângulo tinha 3 vértices independentes. Um cubo tem 6 faces, cada face 2 triângulos, cada triângulo 3 vértices — seriam 36 vértices com muita repetição.

O **index buffer** permite reutilizar vértices. Em vez de duplicar os dados, a GPU lê um índice e busca o vértice correspondente no vertex buffer:

```
Vertex Buffer:  [ v0, v1, v2, v3 ]   (4 vértices únicos por face)
Index Buffer:   [ 0, 1, 2,  0, 2, 3 ] (2 triângulos usando os 4 vértices)
```

Neste projeto cada face tem 4 vértices próprios (cor diferente por face) e 6 índices:

```cpp
// Face frente — 2 triângulos clockwise
0, 1, 2,   // triângulo superior direito
0, 2, 3    // triângulo inferior esquerdo
```

Total: **24 vértices** (6 faces × 4) e **36 índices** (6 faces × 6).

O draw call muda de `Draw()` para `DrawIndexed()`:

```cpp
contexto->DrawIndexed(36, 0, 0); // 36 índices, offset 0, base vertex 0
```

---

### Matrizes de Transformação — MVP

Todo objeto 3D passa por três transformações antes de aparecer na tela:

```
Objeto (local space)
    × matrizMundo      → World Space   (posição no mundo)
    × matrizVisao      → View Space    (relativo à câmera)
    × matrizProjecao   → Clip Space    (perspectiva aplicada)
         ↓
   Pixel na tela
```

#### Matriz Mundo (World Matrix)

Define **onde e como** o objeto está no mundo: posição, rotação e escala.

```cpp
// Rotação em torno de Y e X combinada por multiplicação de matrizes
matrizMundo = XMMatrixRotationY(angulo) * XMMatrixRotationX(angulo * 0.5f);
```

A ordem importa: `Escala × Rotação × Translação` é a convenção padrão.

#### Matriz Visão (View Matrix)

Define a posição e orientação da câmera. `LookAtLH` cria a matriz a partir de três vetores:

```cpp
matrizVisao = XMMatrixLookAtLH(
    XMVectorSet(0.0f, 1.5f, -4.0f, 0.0f), // posição da câmera (atrás e acima)
    XMVectorSet(0.0f, 0.0f,  0.0f, 0.0f), // ponto que a câmera olha (origem)
    XMVectorSet(0.0f, 1.0f,  0.0f, 0.0f)  // vetor "para cima" (eixo Y)
);
```

`LH` = Left-Handed — sistema de coordenadas do DirectX, onde Z positivo aponta para dentro da tela.

#### Matriz Projeção (Projection Matrix)

Aplica a perspectiva: objetos distantes parecem menores. Definida pelo campo de visão (FOV), aspect ratio e planos near/far:

```cpp
matrizProjecao = XMMatrixPerspectiveFovLH(
    XM_PIDIV4,   // FOV vertical: 45 graus (π/4 radianos)
    1280.0f / 720.0f, // aspect ratio: largura/altura
    0.1f,        // plano near: objetos mais próximos que isso são cortados
    100.0f       // plano far:  objetos mais distantes que isso são cortados
);
```

---

### Constant Buffer

O constant buffer é o mecanismo para enviar dados uniformes da CPU para os shaders a cada frame. Todos os vértices de um draw call recebem os mesmos valores.

#### Alinhamento de 16 bytes — regra crítica

O HLSL exige que cada membro de um `cbuffer` comece em um múltiplo de 16 bytes. O struct C++ precisa respeitar isso:

```cpp
struct __declspec(align(16)) DadosConstantes {
    XMMATRIX matrizMundo;    // 64 bytes
    XMMATRIX matrizVisao;    // 64 bytes
    XMMATRIX matrizProjecao; // 64 bytes
};
// Total: 192 bytes — múltiplo de 16 ✓
```

`XMMATRIX` já tem 64 bytes (4×4 floats de 4 bytes). O `__declspec(align(16))` garante que o struct começa em endereço alinhado.

#### Atualização frame a frame com Map/Unmap

O constant buffer usa `D3D11_USAGE_DYNAMIC` + `D3D11_CPU_ACCESS_WRITE` para permitir atualização da CPU:

```cpp
D3D11_MAPPED_SUBRESOURCE msr;
contexto->Map(bufferConstante.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &msr);

DadosConstantes* dados = reinterpret_cast<DadosConstantes*>(msr.pData);
dados->matrizMundo    = XMMatrixTranspose(mundo);
dados->matrizVisao    = XMMatrixTranspose(visao);
dados->matrizProjecao = XMMatrixTranspose(projecao);

contexto->Unmap(bufferConstante.Get(), 0);
```

`MAP_WRITE_DISCARD` descarta o conteúdo anterior — adequado para dados que mudam todo frame.

---

### XMMatrixTranspose — armadilha clássica

DirectXMath armazena matrizes em **row-major** (linha por linha) na memória da CPU. O HLSL espera matrizes em **column-major** (coluna por coluna).

Se não transpormos, a multiplicação `mul(pos, matriz)` no shader produz resultado incorreto — o cubo vai aparecer distorcido ou invisível.

```cpp
// OBRIGATÓRIO antes de enviar qualquer matriz para o shader
dados->matrizMundo = XMMatrixTranspose(mundo);
```

---

### Delta Time

O delta time é o tempo decorrido entre dois frames consecutivos. Usado para que a velocidade de rotação seja independente da taxa de frames:

```cpp
// No loop principal
auto agora       = Relogio::now();
float deltaTempo = std::chrono::duration<float>(agora - ultimoTempo).count();
ultimoTempo      = agora;

// Na atualização
anguloRotacao += deltaTempo * 1.0f; // 1 radiano/segundo em qualquer FPS
```

Sem delta time, a rotação seria mais rápida em máquinas com FPS alto e mais lenta em FPS baixo.

---

### Vertex Shader atualizado

O shader agora aplica as três matrizes sequencialmente via `mul()`:

```hlsl
cbuffer ConstantBuffer : register(b0) {
    matrix matrizMundo;
    matrix matrizVisao;
    matrix matrizProjecao;
};

SaidaVS main(EntradaVS entrada) {
    float4 pos = float4(entrada.posicao, 1.0f);
    pos = mul(pos, matrizMundo);     // local → world space
    pos = mul(pos, matrizVisao);     // world → view space
    pos = mul(pos, matrizProjecao);  // view  → clip space
    ...
}
```

`register(b0)` indica que o constant buffer está no slot 0, correspondendo ao `VSSetConstantBuffers(0, ...)` no C++.

---

### Geometria do Cubo

24 vértices (4 por face, com cor por face) e 36 índices. Cada face é um quad dividido em 2 triângulos clockwise:

```
Face frente (vértices 0-3):

  0 ──── 1
  │  ╲   │
  │    ╲ │
  3 ──── 2

Triângulo 1: 0, 1, 2  (superior direito)
Triângulo 2: 0, 2, 3  (inferior esquerdo)
```

Cores por face: vermelho (frente), verde (trás), azul (cima), amarelo (baixo), magenta (direita), ciano (esquerda).

---

## Diferenças DirectX vs OpenGL nesta etapa

| Aspecto | OpenGL | DirectX 11 |
|---|---|---|
| Uniform/Constant buffer | `glUniformMatrix4fv()` | `Map/Unmap` no constant buffer |
| Transpose das matrizes | GLM já é column-major, sem transpose | `XMMatrixTranspose` obrigatório |
| Sistema de coordenadas | Right-Handed (Z negativo = frente) | Left-Handed (Z positivo = frente) |
| FOV | Pode ser vertical ou horizontal | `PerspectiveFovLH` = vertical |
| Index buffer | `glDrawElements()` | `DrawIndexed()` |
| Câmera | `glm::lookAt()` (Right-Handed) | `XMMatrixLookAtLH()` (Left-Handed) |

---

## Resultado

Cubo 3D com 6 faces de cores distintas rotacionando suavemente em torno dos eixos Y e X. A perspectiva e o depth test garantem que as faces traseiras fiquem ocultas pelas fronteiras.

---

## Próxima etapa

**Etapa 4 — Sistema de Câmera FPS**

Introduz:
- Câmera interativa com posição e direção dinâmicas
- Movimentação com W/A/S/D
- Rotação com mouse ou teclas de seta
- Yaw e Pitch com clamp para evitar inversão
