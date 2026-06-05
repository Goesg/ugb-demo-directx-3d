// Dados de entrada: interpolados pelo rasterizador a partir dos vértices
struct EntradaPS {
    float4 posicao : SV_POSITION;
    float4 cor     : COLOR;
};

// Retorna a cor final do pixel
float4 main(EntradaPS entrada) : SV_TARGET {
    return entrada.cor;
}
