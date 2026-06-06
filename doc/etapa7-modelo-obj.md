# Etapa 7 — Carregamento de Modelo OBJ

## Objetivo

Substituir a geometria hardcoded do cubo por um modelo 3D externo carregado de um arquivo `.obj`, usando a biblioteca TinyObjLoader e reindexando os vértices para o formato esperado pelo DirectX 11.

---

## O que foi implementado

### Arquivos criados

| Arquivo | Responsabilidade |
|---|---|
| `src/Malha.h / .cpp` | Encapsula vertex/index buffers genéricos reutilizáveis |
| `src/Modelo.h / .cpp` | Carrega OBJ via TinyObjLoader, cria Malha e Textura |

### Arquivos modificados

| Arquivo | O que mudou |
|---|---|
| `src/Renderizador.h / .cpp` | Novo método `desenharModelo()` genérico |
| `src/Aplicacao.h / .cpp` | Substituído cubo por `Modelo`, carregamento do OBJ |
| `Demo3D.vcxproj` | Malha e Modelo adicionados ao projeto |

---

## Dependência externa — TinyObjLoader

Antes de compilar, baixe o arquivo `tiny_obj_loader.h`:

**Download:** https://raw.githubusercontent.com/tinyobjloader/tinyobjloader/release/tiny_obj_loader.h

Salve em: `external/tinyobjloader/tiny_obj_loader.h`

O `#define TINYOBJLOADER_IMPLEMENTATION` em `Modelo.cpp` ativa a implementação.

---

## Conceitos Introduzidos

### Formato OBJ — Estrutura do arquivo

O formato OBJ armazena posições, normais e UVs em listas separadas. Cada face referencia combinações desses índices:

```
v  0.0  1.0  0.0       # posição 1
v  1.0 -1.0  0.0       # posição 2
vn 0.0  0.0 -1.0       # normal 1
vt 0.0  0.0            # UV 1
vt 1.0  0.0            # UV 2

f 1/1/1  2/2/1  ...    # face: pos/uv/normal
```

---

### O Problema dos Índices Separados

No OBJ, posição, UV e normal têm índices **independentes**. Uma face pode usar:
- posição 5 + UV 2 + normal 8 para um vértice
- posição 5 + UV 7 + normal 8 para outro

O DirectX 11 usa um **único índice por vértice completo**. É necessário converter cada combinação única `(pos/uv/normal)` em um vértice distinto.

#### Algoritmo de reindexação

```cpp
std::unordered_map<std::string, UINT> mapeamento;

for (cada índice do OBJ) {
    chave = "pos/uv/normal"  // string única para esta combinação

    if (chave não existe no mapa) {
        criar novo Vertice com os dados das listas
        adicionar ao vetor de vértices
        mapeamento[chave] = índice do novo vértice
    }

    indices.push_back(mapeamento[chave])
}
```

Isso garante que vértices idênticos sejam reutilizados (sem duplicatas) e que vértices que compartilham posição mas têm UV/normal diferentes sejam tratados como distintos.

---

### Inversão do eixo V

O OBJ usa origem de UV no **canto inferior esquerdo** (V=0 na base). O DirectX usa origem no **canto superior esquerdo** (V=0 no topo). Sem a inversão, a textura apareceria de cabeça para baixo:

```cpp
v.uv = { attrib.texcoords[ti],
         1.0f - attrib.texcoords[ti + 1] }; // inverter V
```

---

### Módulo Malha

A `Malha` é uma abstração genérica que encapsula os buffers GPU de qualquer geometria:

```cpp
bool Malha::criar(ID3D11Device* device,
                  const std::vector<Vertice>& vertices,
                  const std::vector<UINT>& indices);

void Malha::bind(ID3D11DeviceContext* contexto) const;
```

O método `bind()` configura o Input Assembler para usar esta geometria, desacoplando essa responsabilidade do `Renderizador`.

---

### Módulo Modelo

Agrupa `Malha` + `Textura` + world matrix em uma entidade coesa:

```cpp
modelo.carregar(device, contexto, "assets/models/modelo.obj", "assets/textures/textura.png");
modelo.definirMatrizMundo(XMMatrixRotationY(angulo));
renderizador->desenharModelo(modelo.obterMalha(), modelo.obterTextura(), ...);
```

---

### Método `desenharModelo` no Renderizador

Reutiliza exatamente o mesmo pipeline de shaders da Etapa 6 — o pipeline não sabe nem se importa se a geometria veio de um cubo hardcoded ou de um OBJ. Apenas a fonte dos buffers muda:

```cpp
malha.bind(contexto.Get());                              // geometria do modelo
contexto->DrawIndexed(malha.obterNumIndices(), 0, 0);   // quantidade variável de índices
```

---

## Preparação para testar

**1. Baixar TinyObjLoader**

```powershell
Invoke-WebRequest -Uri "https://raw.githubusercontent.com/tinyobjloader/tinyobjloader/release/tiny_obj_loader.h" -OutFile "external\tinyobjloader\tiny_obj_loader.h"
```

**2. Adicionar um modelo OBJ**

Coloque um arquivo `.obj` em `assets/models/modelo.obj`. Sugestões de modelos gratuitos:
- Qualquer modelo simples exportado do Blender (File → Export → Wavefront OBJ)
- Sites como [sketchfab.com](https://sketchfab.com) (filtrar por formato OBJ, licença free)

O modelo deve ter normais e UVs exportados. No Blender: marcar **Write Normals** e **Include UVs** na exportação.

**3. Compilar e executar**

```powershell
msbuild Demo3D.vcxproj /p:Configuration=Debug /p:Platform=x64
.\build\x64\Debug\Demo3D.exe
```

---

## Diferenças DirectX vs OpenGL nesta etapa

| Aspecto | OpenGL | DirectX 11 |
|---|---|---|
| Carregar OBJ | Qualquer lib (Assimp, tinyobj) | Igual — independente da API gráfica |
| Reindexação | Necessária da mesma forma | Necessária da mesma forma |
| Inversão UV | Necessária (origens opostas) | Necessária (inverter V) |
| `DrawElements` | `glDrawElements(GL_TRIANGLES, n, ...)` | `DrawIndexed(n, 0, 0)` |

---

## Resultado

O modelo OBJ externo é renderizado na cena com textura e iluminação direcional, usando os mesmos shaders das etapas anteriores. A câmera FPS permite navegar ao redor do modelo.

---

## MVP Concluído

Com esta etapa, todos os requisitos obrigatórios do projeto estão implementados:

| Requisito | Status |
|---|---|
| Janela Win32 | ✅ Etapa 1 |
| DirectX 11 inicializado | ✅ Etapa 1 |
| Swap chain + Render target + Depth buffer | ✅ Etapa 1 |
| Vertex Shader + Pixel Shader (HLSL) | ✅ Etapa 2 |
| Index buffer + Matrizes MVP | ✅ Etapa 3 |
| Câmera FPS interativa (WASD + setas) | ✅ Etapa 4 |
| Texturização com stb_image | ✅ Etapa 5 |
| Iluminação direcional difusa | ✅ Etapa 6 |
| Carregamento de modelo OBJ externo | ✅ Etapa 7 |
