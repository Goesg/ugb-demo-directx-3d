// Constant buffer com as matrizes de transformação (enviadas pela CPU a cada frame)
cbuffer ConstantBuffer : register(b0) {
    matrix matrizMundo;      // posição/rotação/escala do objeto no mundo
    matrix matrizVisao;      // posição e orientação da câmera
    matrix matrizProjecao;   // perspectiva (FOV, aspect ratio, near/far)
};

struct EntradaVS {
    float3 posicao : POSITION;
    float4 cor     : COLOR;
};

struct SaidaVS {
    float4 posicao : SV_POSITION;
    float4 cor     : COLOR;
};

SaidaVS main(EntradaVS entrada) {
    SaidaVS saida;

    // Aplicar as três transformações em sequência: Mundo → Visão → Projeção
    float4 pos = float4(entrada.posicao, 1.0f);
    pos = mul(pos, matrizMundo);
    pos = mul(pos, matrizVisao);
    pos = mul(pos, matrizProjecao);

    saida.posicao = pos;
    saida.cor     = entrada.cor;
    return saida;
}
