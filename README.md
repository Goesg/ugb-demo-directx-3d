# Demo 3D DirectX 11

Projeto acadêmico de Computação Gráfica — Tech Demo interativo 3D desenvolvido com DirectX 11 e C++.

---

## Tecnologias Utilizadas

- C++ 17
- DirectX 11 (D3D11)
- Win32 API
- HLSL Shaders
- Visual Studio 2022
- Windows SDK 10.0

### Bibliotecas Externas

- [TinyObjLoader](https://github.com/tinyobjloader/tinyobjloader) — carregamento de modelos OBJ
- [stb_image](https://github.com/nothings/stb) — carregamento de texturas
- DirectXMath — operações matemáticas vetoriais e matriciais

---

## Pré-requisitos

- Windows 10/11
- Visual Studio 2022 com workload **Desenvolvimento para Desktop com C++**
- Windows SDK 10.0 ou superior

---

## Como Compilar

**Opção 1 — Visual Studio:**
1. Abra o arquivo `Demo3D.vcxproj` no Visual Studio 2022
2. Selecione a configuração `Debug | x64`
3. Pressione `Ctrl+Shift+B` para compilar

**Opção 2 — Linha de comando (Developer PowerShell for VS):**
```powershell
.\compilar.bat
```

---

## Como Executar

```powershell
.\build\x64\Debug\Demo3D.exe
```

Ou pressione `F5` no Visual Studio para rodar com depuração.

> O executável deve ser rodado a partir da raiz do projeto para que os assets (modelos e texturas) sejam encontrados corretamente.

---

## Controles

### Câmera

| Tecla | Ação |
|---|---|
| `W` | Mover câmera para frente |
| `S` | Mover câmera para trás |
| `A` | Mover câmera para esquerda |
| `D` | Mover câmera para direita |
| `Seta esquerda / direita` | Rotacionar câmera horizontalmente (yaw) |
| `Seta cima / baixo` | Rotacionar câmera verticalmente (pitch) |

### Objeto — Rotação

| Tecla / Input | Ação |
|---|---|
| `R` | Cicla o eixo de rotação: Y → X → Z → Combinado |
| `Clique esquerdo do mouse` | Pausa / retoma a rotação do objeto |

### Objeto — Posição

| Tecla | Ação |
|---|---|
| `Numpad 4` | Mover objeto para esquerda (eixo X−) |
| `Numpad 6` | Mover objeto para direita (eixo X+) |
| `Numpad 8` | Mover objeto para frente (eixo Z−) |
| `Numpad 2` | Mover objeto para trás (eixo Z+) |
| `Numpad 9` | Mover objeto para cima (eixo Y+) |
| `Numpad 3` | Mover objeto para baixo (eixo Y−) |

### Objeto — Escala

| Tecla | Ação |
|---|---|
| `=` | Aumentar escala do objeto |
| `-` | Diminuir escala do objeto |

### Cena

| Tecla | Ação |
|---|---|
| `T` | Alternar textura (cicla pelos arquivos em `assets/textures/`) |
| `M` | Alternar modelo 3D (cicla pelos arquivos em `assets/models/`) |
| `ESC` | Fechar a aplicação |

> Ao trocar de modelo com `M`, posição, escala e ângulo de rotação são resetados automaticamente.

---

## Adicionando Conteúdo

### Novos modelos

Coloque arquivos `.obj` em `assets/models/`. O Demo carrega todos automaticamente na inicialização e os disponibiliza via `M`. O modelo deve ter normais e UVs exportados.

### Novas texturas

Coloque arquivos `.png` em `assets/textures/`. O Demo carrega todas automaticamente na inicialização e as disponibiliza via `T`.

> Modelos e texturas são carregados em **ordem alfabética** pelo nome do arquivo.

---

## Funcionalidades Implementadas

- [x] Janela Win32
- [x] Inicialização DirectX 11
- [x] Swap Chain + Render Target + Depth Buffer
- [x] Render loop com delta time
- [x] Triângulo e cubo 3D (geometria hardcoded)
- [x] Câmera FPS interativa (WASD + setas)
- [x] Texturização com stb_image
- [x] Iluminação direcional difusa (Lambert)
- [x] Carregamento de modelos OBJ externos (TinyObjLoader)
- [x] Alternância de texturas em runtime (tecla T)
- [x] Alternância de modelos em runtime (tecla M)
- [x] Ciclo de eixo de rotação (tecla R)
- [x] Pausa da rotação com clique do mouse
- [x] Movimentação do objeto no espaço 3D (Numpad)
- [x] Escala do objeto em runtime (= / -)

---

## Estrutura do Projeto

```
/
├── src/
│   ├── main.cpp           → ponto de entrada WinMain
│   ├── Aplicacao.h/cpp    → orquestrador: loop, entrada, cena
│   ├── Janela.h/cpp       → janela Win32
│   ├── Renderizador.h/cpp → pipeline DirectX 11, draw calls
│   ├── Camera.h/cpp       → câmera FPS
│   ├── Modelo.h/cpp       → carregamento OBJ
│   ├── Malha.h/cpp        → vertex/index buffers
│   └── Textura.h/cpp      → carregamento de imagens PNG
├── shaders/
│   ├── VertexShader.hlsl  → transformação MVP + normais
│   └── PixelShader.hlsl   → texturização + iluminação
├── assets/
│   ├── models/            → modelos OBJ (adicione aqui)
│   └── textures/          → texturas PNG (adicione aqui)
├── external/
│   ├── tinyobjloader/     → TinyObjLoader (header-only)
│   └── stb/               → stb_image (header-only)
├── doc/                   → documentação técnica por etapa
├── build/                 → binários gerados (ignorado no git)
├── compilar.bat           → script de build via linha de comando
└── Demo3D.vcxproj         → projeto Visual Studio
```

---

## Equipe

- Diego Goes
- Gutemberg
- Júlio
- Hugo
- Thomas

---

## Screenshots

*(a ser adicionado)*
