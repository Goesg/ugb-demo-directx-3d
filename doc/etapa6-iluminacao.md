# Etapa 6 — Iluminação Direcional

## Objetivo

Adicionar iluminação difusa direcional ao cubo texturizado, fazendo com que as faces voltadas para a luz fiquem claras e as faces opostas fiquem escuras, usando o modelo de iluminação Lambert.

---

## O que foi implementado

### Arquivos modificados

| Arquivo | O que mudou |
|---|---|
| `shaders/VertexShader.hlsl` | Normal adicionada à entrada, transformada para world space na saída |
| `shaders/PixelShader.hlsl` | `cbuffer` de luz, cálculo difusa + ambiente, modulação da textura |
| `src/Renderizador.h` | `Vertice` com `normal`, struct `DadosLuz`, `bufferLuz` |
| `src/Renderizador.cpp` | Normal no Input Layout, normais no cubo, atualização do `bufferLuz` |
| `src/Aplicacao.h` | Membro `DadosLuz luz` com valores iniciais |
| `src/Aplicacao.cpp` | Passagem de `luz` para `desenharCubo()` |

---

## Conceitos Introduzidos

### Modelo de Iluminação Lambert (Diffuse)

A iluminação difusa baseia-se na lei de Lambert: a intensidade de luz em uma superfície é proporcional ao cosseno do ângulo entre a normal da superfície e a direção da luz.

```
Iluminação difusa = max(0, dot(normal, -direcaoLuz))
```

- `normal` = vetor perpendicular à face (normalizado)
- `direcaoLuz` = vetor apontando **de onde vem** a luz (ex: de cima e da esquerda)
- O sinal negativo em `-direcaoLuz` inverte o vetor para apontar **em direção à fonte**
- `saturate()` = clamp entre 0 e 1 (equivale ao `max(0, x)`)

```
face voltada para a luz → dot ≈ 1.0 → brilhante
face perpendicular       → dot ≈ 0.0 → sem luz direta
face oposta              → dot < 0.0 → saturado em 0 (ambiente cobre)
```

---

### Componente Ambiente

Sem a luz ambiente, as faces opostas à luz seriam totalmente pretas — pouco realista. A componente ambiente adiciona um mínimo de iluminação a todas as faces:

```hlsl
float iluminacao = ambiente + difusa * intensidade;
// ex: 0.15 + 0.85 * 1.0 = 1.0 (face totalmente iluminada)
// ex: 0.15 + 0.0  * 1.0 = 0.15 (face na sombra, 15% de brilho)
```

O valor `0.15f` garante que nenhuma face fique completamente preta.

---

### Normais no Vertex Buffer

Cada vértice agora carrega a normal da sua face. Para um cubo, as normais são axiais (apontam exatamente para um dos 6 eixos):

```cpp
// Face frente — normal aponta em -Z (para o observador)
{ XMFLOAT3(-0.5f, 0.5f, -0.5f), XMFLOAT3(0, 0, -1), XMFLOAT2(0, 0) },
```

```
Face frente  → normal ( 0,  0, -1)
Face trás    → normal ( 0,  0,  1)
Face cima    → normal ( 0,  1,  0)
Face baixo   → normal ( 0, -1,  0)
Face direita → normal ( 1,  0,  0)
Face esquerda→ normal (-1,  0,  0)
```

---

### Transformação da Normal para World Space

A normal precisa ser transformada junto com o objeto quando ele rotaciona. Caso contrário, a iluminação ficaria "pregada" ao objeto independente da rotação.

```hlsl
saida.normalMundo = normalize(mul(float4(entrada.normal, 0.0f), matrizMundo).xyz);
```

**Por que `w = 0`?** Vetores de direção (como normais) não devem ser afetados pela translação da matriz. Usar `w = 0` garante que apenas a rotação/escala seja aplicada (não a translação).

**Nota:** Para objetos com escala não-uniforme, seria necessário usar o **inverso transposto** da matriz mundo. Para escala uniforme (como nosso cubo), a matriz mundo direta funciona corretamente.

---

### Constant Buffer de Luz — Alinhamento

O `DadosLuz` deve ter tamanho múltiplo de 16 bytes. O `float3` ocupa 12 bytes, então um `float` após ele completa os 16:

```cpp
struct __declspec(align(16)) DadosLuz {
    XMFLOAT3 direcaoLuz;  // 12 bytes
    float    intensidade; //  4 bytes → total 16 ✓
    XMFLOAT3 corLuz;      // 12 bytes
    float    ambiente;    //  4 bytes → total 16 ✓
};
```

Se os membros não estivessem nessa ordem, haveria padding implícito e os valores chegariam errados ao shader.

---

### Dois Constant Buffers em paralelo

O vertex shader usa o slot `b0` (matrizes MVP) e o pixel shader usa o slot `b1` (dados de luz):

```cpp
// b0 → vertex shader
contexto->VSSetConstantBuffers(0, 1, bufferConstante.GetAddressOf());

// b1 → pixel shader
contexto->PSSetConstantBuffers(1, 1, bufferLuz.GetAddressOf());
```

No HLSL, os registros correspondem:

```hlsl
cbuffer ConstantBuffer : register(b0) { ... } // vertex shader
cbuffer BufferLuz      : register(b1) { ... } // pixel shader
```

---

### Pixel Shader completo

```hlsl
float4 main(EntradaPS entrada) : SV_TARGET {
    float difusa    = saturate(dot(normalize(entrada.normalMundo), -direcaoLuz));
    float iluminacao = ambiente + difusa * intensidade;

    float4 corTextura = textura.Sample(amostrador, entrada.uv);
    return float4(corTextura.rgb * corLuz * iluminacao, corTextura.a);
}
```

A cor final é a textura modulada pela iluminação e pela cor da luz. `corLuz = (1,1,1)` = luz branca, que não altera a cor da textura.

---

## Configuração da luz padrão

```cpp
DadosLuz luz = {
    XMFLOAT3(0.5f, -0.8f, 0.3f), // vem de cima, levemente à direita e à frente
    1.0f,                          // intensidade total
    XMFLOAT3(1.0f, 1.0f, 1.0f),  // luz branca
    0.15f                          // 15% de luz ambiente
};
```

---

## Diferenças DirectX vs OpenGL nesta etapa

| Aspecto | OpenGL | DirectX 11 |
|---|---|---|
| Enviar dados de luz | `glUniform3f()` / UBO | `Map/Unmap` no constant buffer |
| Normal no shader | `in vec3 normal` (GLSL) | `float3 normal : NORMAL` (HLSL) |
| `dot()` | `dot(a, b)` (GLSL) | `dot(a, b)` (HLSL — igual) |
| `clamp(x,0,1)` | `clamp(x, 0.0, 1.0)` | `saturate(x)` |
| Múltiplos UBOs | `glBindBufferBase(GL_UNIFORM_BUFFER, slot, ...)` | `VSSetConstantBuffers(slot, ...)` |

---

## Resultado

O cubo texturizado agora exibe variação de brilho por face conforme a orientação em relação à luz. Faces voltadas para a fonte ficam na intensidade máxima; faces opostas ficam com apenas 15% de brilho (ambiente). A rotação do cubo faz com que o shading mude dinamicamente.

---

## Próxima etapa

**Etapa 7 — Carregamento de Modelo OBJ**

Introduz:
- Integração do TinyObjLoader
- Parsing de arquivo `.obj` externo
- Reindexação de vértices (OBJ usa índices separados por atributo)
- Substituição do cubo hardcoded por geometria carregada do disco
