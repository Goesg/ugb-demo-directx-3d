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

1. Abra o arquivo `Demo3D.vcxproj` no Visual Studio 2022
2. Selecione a configuração `Debug | x64`
3. Pressione `Ctrl+Shift+B` para compilar

---

## Como Executar

Após compilar, execute `build/x64/Debug/Demo3D.exe`

Ou pressione `F5` no Visual Studio para rodar com depuração.

---

## Controles

| Tecla | Ação |
|---|---|
| `W` | Mover para frente |
| `S` | Mover para trás |
| `A` | Mover para esquerda |
| `D` | Mover para direita |
| Mouse / Setas | Rotacionar câmera |
| `ESC` | Fechar aplicação |

---

## Funcionalidades Implementadas

- [x] Janela Win32
- [x] Inicialização DirectX 11
- [x] Swap Chain + Render Target + Depth Buffer
- [x] Render loop com clear color
- [ ] Triângulo colorido (Etapa 2)
- [ ] Cubo 3D (Etapa 3)
- [ ] Câmera FPS (Etapa 4)
- [ ] Texturização (Etapa 5)
- [ ] Iluminação direcional (Etapa 6)
- [ ] Carregamento de modelo OBJ (Etapa 7)

---

## Estrutura do Projeto

```
/
├── src/            → código-fonte C++
├── shaders/        → shaders HLSL
├── assets/         → modelos e texturas
├── external/       → bibliotecas externas (header-only)
├── build/          → binários gerados (ignorado no git)
└── Demo3D.vcxproj  → projeto Visual Studio
```

---

## Equipe

- Diego Goes

---

## Screenshots

*(a ser adicionado após Etapa 1)*
