# Etapa 1 — Janela Win32 e Inicialização DirectX 11

## Objetivo

Criar a fundação do projeto: uma janela Win32 funcional com o DirectX 11 inicializado, capaz de executar um render loop que limpa a tela com uma cor sólida a cada frame.

---

## O que foi implementado

### Módulos criados

| Arquivo | Responsabilidade |
|---|---|
| `src/Janela.h / .cpp` | Criação e gerenciamento da janela Win32 |
| `src/Renderizador.h / .cpp` | Inicialização do DirectX 11 e pipeline de renderização |
| `src/Aplicacao.h / .cpp` | Orquestrador central: inicializa módulos e executa o loop |
| `src/main.cpp` | Entry point da aplicação (`WinMain`) |
| `Demo3D.vcxproj` | Projeto Visual Studio configurado para compilar o projeto |
| `compilar.bat` | Script para compilar e rodar via duplo clique no Windows |

---

## Explicação dos Módulos

### Janela (`Janela.h / Janela.cpp`)

Responsável por criar e gerenciar a janela do sistema operacional usando a **Win32 API**.

**O que faz:**
- Registra uma classe de janela (`WNDCLASSEXW`) com ícone, cursor e procedimento de mensagens
- Cria a janela com `CreateWindowExW` no tamanho 1280×720
- Processa mensagens do sistema operacional em loop (`PeekMessage`)
- Fecha a aplicação quando o usuário pressiona **ESC** ou fecha a janela

**Conceito importante — WndProc:**
O Windows se comunica com a aplicação enviando mensagens (clique, teclado, fechar, etc.). A função `WndProc` é o "receptor" dessas mensagens:

```cpp
LRESULT CALLBACK Janela::WndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_KEYDOWN:
            if (wParam == VK_ESCAPE) PostQuitMessage(0); // ESC fecha
            break;
        case WM_DESTROY:
            PostQuitMessage(0); // botão X fecha
            break;
        ...
    }
}
```

---

### Renderizador (`Renderizador.h / Renderizador.cpp`)

Responsável por toda a inicialização do DirectX 11 e pelos comandos de renderização frame a frame.

#### Objetos DirectX criados

| Objeto | Tipo | Para que serve |
|---|---|---|
| Device | `ID3D11Device` | Cria recursos na GPU (buffers, texturas, shaders) |
| Contexto | `ID3D11DeviceContext` | Envia comandos de renderização para a GPU |
| SwapChain | `IDXGISwapChain` | Gerencia os buffers de exibição (front/back buffer) |
| RenderTargetView | `ID3D11RenderTargetView` | Define onde a GPU vai desenhar (o back buffer) |
| DepthStencilView | `ID3D11DepthStencilView` | Buffer de profundidade para oclusão correta entre objetos 3D |

#### Fluxo de inicialização

```
D3D11CreateDeviceAndSwapChain()
    ↓
SwapChain->GetBuffer()          → obtém o back buffer
    ↓
Device->CreateRenderTargetView() → permite desenhar no back buffer
    ↓
Device->CreateTexture2D()        → cria textura do depth buffer
    ↓
Device->CreateDepthStencilView() → permite usar depth test
    ↓
Contexto->OMSetRenderTargets()   → vincula RTV + DSV ao pipeline
    ↓
Contexto->RSSetViewports()       → define área de renderização
```

#### Por que o Device e o DeviceContext são separados?

No DirectX 11, a criação de recursos e a execução de comandos são responsabilidades distintas:

- **`ID3D11Device`** → fábrica de recursos (cria buffers, texturas, shaders). Usado raramente após inicialização.
- **`ID3D11DeviceContext`** → executor de comandos (limpar tela, desenhar, configurar pipeline). Usado a cada frame.

#### SwapChain — Double Buffering

A tela nunca exibe o frame que está sendo desenhado. Existe um **back buffer** (onde a GPU desenha) e um **front buffer** (o que o monitor exibe). Quando o frame está pronto, `Present()` troca os dois:

```
Frame N sendo desenhado → back buffer
Frame N-1 sendo exibido → front buffer
                        ↓
         swapChain->Present(1, 0)
                        ↓
Frame N passa a ser o front buffer (exibido)
Frame N+1 começa a ser desenhado no back buffer
```

#### Depth Buffer

Sem o depth buffer, a GPU não sabe qual objeto está "na frente". Cada pixel tem um valor de profundidade entre `0.0` (mais perto) e `1.0` (mais longe). O depth test descarta pixels que estão atrás de algo já desenhado.

```cpp
// Limpar o depth buffer a cada frame (resetar todos os valores para 1.0)
contexto->ClearDepthStencilView(depthStencilView.Get(),
                                D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL,
                                1.0f, 0);
```

---

### Aplicacao (`Aplicacao.h / Aplicacao.cpp`)

Orquestra todos os módulos e executa o loop principal da aplicação.

**Loop principal:**

```cpp
void Aplicacao::executar() {
    while (janela->processarMensagens()) {  // retorna false quando fecha
        atualizar();   // lógica do jogo (vazio na Etapa 1)
        renderizar();  // limpar tela + Present
    }
}
```

**Renderização da Etapa 1:**

```cpp
void Aplicacao::renderizar() {
    renderizador->limparTela(0.05f, 0.08f, 0.15f); // azul escuro
    renderizador->apresentar();                      // Present
}
```

---

### main.cpp

Entry point de aplicações Windows. `WinMain` é o equivalente ao `main()` normal, mas recebido pelo Windows:

```cpp
int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    Aplicacao app;
    if (!app.inicializar()) {
        MessageBoxW(...); // exibe erro se falhar
        return -1;
    }
    app.executar();
    return 0;
}
```

---

## Gerenciamento de Memória — ComPtr

Objetos DirectX são interfaces **COM** (Component Object Model). Normalmente precisariam de `->Release()` manual para liberar a memória da GPU. Para evitar vazamentos, usamos `ComPtr<>` (smart pointer da Microsoft):

```cpp
ComPtr<ID3D11Device> device; // libera automaticamente quando sai de escopo
```

Equivalente ao `std::unique_ptr`, mas para objetos COM.

---

## Diferenças DirectX vs OpenGL nesta etapa

| Aspecto | OpenGL | DirectX 11 |
|---|---|---|
| Inicialização | `glfwInit()` + `gladLoadGL()` (simples) | Cadeia de ~10 objetos COM |
| Contexto | Implícito, ligado à thread | Explícito (`ID3D11DeviceContext`) |
| Swap buffers | `glfwSwapBuffers()` | `swapChain->Present(1, 0)` |
| Depth buffer | Habilitado com `glEnable(GL_DEPTH_TEST)` | Criado e vinculado manualmente |
| Gerenciamento de memória | Automático via contexto | Manual via COM / ComPtr |

---

## Resultado

Ao executar `Demo3D.exe`, uma janela **1280×720** é aberta com fundo **azul escuro** renderizado pelo DirectX 11. A janela fecha ao pressionar **ESC** ou o botão X.

O frame rate é limitado pelo VSync (`Present(1, 0)`), estável em ~60fps.

---

## Arquivos modificados nesta etapa

```
src/main.cpp
src/Aplicacao.h
src/Aplicacao.cpp
src/Janela.h
src/Janela.cpp
src/Renderizador.h
src/Renderizador.cpp
Demo3D.vcxproj
compilar.bat
.gitignore
README.md
```

---

## Próxima etapa

**Etapa 2 — Triângulo Colorido**

Introduz os conceitos fundamentais do pipeline gráfico:
- Vertex Buffer
- Input Layout
- Vertex Shader (HLSL)
- Pixel Shader (HLSL)
- Primeiro draw call: `Draw()`
