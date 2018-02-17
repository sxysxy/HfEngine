struct vs_output{
    float4 pos : SV_POSITION;
};

cbuffer param : register(b0){
    float4 color;
};

float4 main(vs_output output) : SV_TARGET{
    return color;
}