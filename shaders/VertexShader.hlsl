cbuffer ConstantBuffer : register(b0) {
    matrix matrizMundo;
    matrix matrizVisao;
    matrix matrizProjecao;
};

struct EntradaVS {
    float3 posicao : POSITION;
    float2 uv      : TEXCOORD0;
};

struct SaidaVS {
    float4 posicao : SV_POSITION;
    float2 uv      : TEXCOORD0;
};

SaidaVS main(EntradaVS entrada) {
    SaidaVS saida;

    float4 pos = float4(entrada.posicao, 1.0f);
    pos = mul(pos, matrizMundo);
    pos = mul(pos, matrizVisao);
    pos = mul(pos, matrizProjecao);

    saida.posicao = pos;
    saida.uv      = entrada.uv;
    return saida;
}
