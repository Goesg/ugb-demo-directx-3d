cbuffer ConstantBuffer : register(b0) {
    matrix matrizMundo;
    matrix matrizVisao;
    matrix matrizProjecao;
};

struct EntradaVS {
    float3 posicao : POSITION;
    float3 normal  : NORMAL;
    float2 uv      : TEXCOORD0;
};

struct SaidaVS {
    float4 posicao       : SV_POSITION;
    float3 normalMundo   : NORMAL;      // normal transformada para world space
    float2 uv            : TEXCOORD0;
};

SaidaVS main(EntradaVS entrada) {
    SaidaVS saida;

    float4 pos = float4(entrada.posicao, 1.0f);
    pos = mul(pos, matrizMundo);
    pos = mul(pos, matrizVisao);
    pos = mul(pos, matrizProjecao);
    saida.posicao = pos;

    // Transformar a normal para world space (apenas rotação — sem translação)
    // Para escala uniforme, usar a matriz mundo diretamente é suficiente
    saida.normalMundo = normalize(mul(float4(entrada.normal, 0.0f), matrizMundo).xyz);

    saida.uv = entrada.uv;
    return saida;
}
