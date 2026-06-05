struct EntradaPS {
    float4 posicao : SV_POSITION;
    float4 cor     : COLOR;
};

float4 main(EntradaPS entrada) : SV_TARGET {
    return entrada.cor;
}
