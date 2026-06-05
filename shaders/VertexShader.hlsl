// Dados de entrada: um vértice vindo do vertex buffer
struct EntradaVS {
    float3 posicao : POSITION;
    float4 cor     : COLOR;
};

// Dados de saída: o vértice transformado para o pixel shader
struct SaidaVS {
    float4 posicao : SV_POSITION; // posição final em clip space
    float4 cor     : COLOR;
};

// Etapa 2: sem transformações ainda — passa a posição diretamente
SaidaVS main(EntradaVS entrada) {
    SaidaVS saida;
    saida.posicao = float4(entrada.posicao, 1.0f);
    saida.cor     = entrada.cor;
    return saida;
}
