Texture2D    textura : register(t0); // slot de textura 0
SamplerState amostrador : register(s0); // slot de sampler 0

struct EntradaPS {
    float4 posicao : SV_POSITION;
    float2 uv      : TEXCOORD0;
};

float4 main(EntradaPS entrada) : SV_TARGET {
    return textura.Sample(amostrador, entrada.uv);
}
