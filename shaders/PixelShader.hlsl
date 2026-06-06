Texture2D    textura    : register(t0);
SamplerState amostrador : register(s0);

// Dados da luz direcional enviados pela CPU
cbuffer BufferLuz : register(b1) {
    float3 direcaoLuz;   // direção de onde a luz vem (normalizada)
    float  intensidade;
    float3 corLuz;       // cor da luz (RGB)
    float  ambiente;     // intensidade mínima de luz (evita faces totalmente pretas)
};

struct EntradaPS {
    float4 posicao     : SV_POSITION;
    float3 normalMundo : NORMAL;
    float2 uv          : TEXCOORD0;
};

float4 main(EntradaPS entrada) : SV_TARGET {
    // Componente difusa: quanto a face está voltada para a luz
    // dot(normal, -direcaoLuz): negativo porque direcaoLuz aponta "de onde vem a luz"
    float difusa = saturate(dot(normalize(entrada.normalMundo), -direcaoLuz));

    // Iluminação final = ambiente + difusa × intensidade
    float iluminacao = ambiente + difusa * intensidade;

    // Cor da textura modulada pela iluminação
    float4 corTextura = textura.Sample(amostrador, entrada.uv);
    return float4(corTextura.rgb * corLuz * iluminacao, corTextura.a);
}
