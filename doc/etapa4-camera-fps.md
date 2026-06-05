# Etapa 4 — Sistema de Câmera FPS

## Objetivo

Substituir a câmera fixa da Etapa 3 por uma câmera interativa no estilo FPS: movimentação com W/A/S/D e rotação com as teclas de seta, gerando a view matrix dinamicamente a cada frame.

---

## O que foi implementado

### Arquivos criados

| Arquivo | Responsabilidade |
|---|---|
| `src/Camera.h` | Declaração da câmera: posição, yaw/pitch, velocidade |
| `src/Camera.cpp` | Leitura de input, cálculo de direção, geração da view matrix |

### Arquivos modificados

| Arquivo | O que mudou |
|---|---|
| `src/Aplicacao.h` | Substituída matriz visão fixa por `std::unique_ptr<Camera>` |
| `src/Aplicacao.cpp` | `camera->processar()` na atualização, matrizes da câmera no draw |
| `Demo3D.vcxproj` | `Camera.cpp` e `Camera.h` adicionados ao projeto |

---

## Conceitos Introduzidos

### Yaw e Pitch

A orientação da câmera é representada por dois ângulos em radianos:

```
Yaw   → rotação horizontal (girar para esquerda/direita)
Pitch → rotação vertical   (olhar para cima/baixo)
```

As setas do teclado incrementam ou decrementam esses ângulos proporcionalmente ao delta time:

```cpp
if (GetAsyncKeyState(VK_LEFT)  & 0x8000) yaw   -= sensibilidade * deltaTempo;
if (GetAsyncKeyState(VK_RIGHT) & 0x8000) yaw   += sensibilidade * deltaTempo;
if (GetAsyncKeyState(VK_UP)    & 0x8000) pitch += sensibilidade * deltaTempo;
if (GetAsyncKeyState(VK_DOWN)  & 0x8000) pitch -= sensibilidade * deltaTempo;
```

`GetAsyncKeyState` retorna o estado atual da tecla independentemente do foco da janela. O bit `0x8000` indica que a tecla está pressionada agora.

---

### Clamp do Pitch

Sem limitação, o pitch poderia ultrapassar ±90°, fazendo a câmera "virar de cabeça para baixo" e inverter os controles. O clamp impede isso:

```cpp
static constexpr float PITCH_MAX = XM_PIDIV2 - 0.01f; // ~89 graus

if (pitch >  PITCH_MAX) pitch =  PITCH_MAX;
if (pitch < -PITCH_MAX) pitch = -PITCH_MAX;
```

`XM_PIDIV2` = π/2 ≈ 1.5708 radianos = 90°. O `- 0.01f` evita a singularidade exata em 90°.

---

### Conversão Yaw/Pitch → Vetor Direção

Os ângulos são convertidos em um vetor de direção 3D usando trigonometria esférica:

```cpp
direcao.x = cos(pitch) * sin(yaw);  // componente horizontal X
direcao.y = sin(pitch);              // componente vertical Y
direcao.z = cos(pitch) * cos(yaw);  // componente de profundidade Z
```

Visualizando:

```
pitch = 0, yaw = 0   → direção (0, 0, 1)  = olhando para frente (+Z)
pitch = 0, yaw = π/2 → direção (1, 0, 0)  = olhando para direita (+X)
pitch = π/4, yaw = 0 → direção (0, 0.7, 0.7) = olhando para cima e frente
```

---

### Movimentação WASD

O movimento usa os vetores `frente` e `direita` calculados a partir da direção atual da câmera:

```cpp
XMVECTOR frente  = XMLoadFloat3(&direcao);
XMVECTOR up      = XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f);
XMVECTOR direita = XMVector3Normalize(XMVector3Cross(frente, up));
```

O vetor `direita` é obtido pelo **produto vetorial** entre `frente` e `up`. O produto vetorial de dois vetores produz um vetor perpendicular a ambos — exatamente o que precisamos para o strafe lateral.

```
frente × up = direita  (perpendicular ao plano formado por frente e up)
```

O `Normalize` garante que `direita` tenha comprimento 1 para que a velocidade seja uniforme.

Aplicando o movimento:

```cpp
float vel = velocidade * deltaTempo;

if (GetAsyncKeyState('W') & 0x8000) pos += frente  *  vel; // frente
if (GetAsyncKeyState('S') & 0x8000) pos += frente  * -vel; // trás
if (GetAsyncKeyState('D') & 0x8000) pos += direita *  vel; // direita
if (GetAsyncKeyState('A') & 0x8000) pos += direita * -vel; // esquerda
```

Note que `'W'`, `'S'`, `'A'`, `'D'` são literais char — `GetAsyncKeyState` aceita o código ASCII da tecla maiúscula.

---

### Geração da View Matrix

A view matrix é recalculada a cada frame a partir da posição e direção atuais:

```cpp
XMMATRIX Camera::obterMatrizVisao() const {
    XMVECTOR pos  = XMLoadFloat3(&posicao);
    XMVECTOR dir  = XMLoadFloat3(&direcao);
    XMVECTOR alvo = XMVectorAdd(pos, dir);      // ponto que a câmera olha
    XMVECTOR up   = XMVectorSet(0, 1, 0, 0);

    return XMMatrixLookAtLH(pos, alvo, up);
}
```

`alvo = posicao + direcao` — o ponto que a câmera está olhando é sempre um passo à frente da posição atual.

---

### Controles

| Tecla | Ação |
|---|---|
| `W` | Mover para frente |
| `S` | Mover para trás |
| `A` | Strafe para esquerda |
| `D` | Strafe para direita |
| `←` `→` | Rotacionar horizontalmente (yaw) |
| `↑` `↓` | Rotacionar verticalmente (pitch) |
| `ESC` | Fechar a aplicação |

---

## Diferenças DirectX vs OpenGL nesta etapa

| Aspecto | OpenGL (típico) | DirectX 11 |
|---|---|---|
| Leitura de input | GLFW (`glfwGetKey`) | Win32 (`GetAsyncKeyState`) |
| View matrix | `glm::lookAt()` (Right-Handed) | `XMMatrixLookAtLH()` (Left-Handed) |
| Eixo Z frente | Z negativo | Z positivo |
| Vetor direita | `cross(frente, up)` | `cross(frente, up)` (igual) |

---

## Resultado

Câmera totalmente interativa: W/A/S/D movem pelo espaço 3D e as setas rotacionam o campo de visão. O cubo continua rotacionando no centro da cena enquanto o jogador se move ao redor.

---

## Próxima etapa

**Etapa 5 — Texturização**

Introduz:
- Carregamento de imagem PNG com `stb_image`
- Criação de `ID3D11Texture2D` e `ID3D11ShaderResourceView`
- `ID3D11SamplerState` para filtragem de textura
- Coordenadas UV no vertex buffer
- `texture.Sample()` no pixel shader
