cbuffer param : register(b0){
	float rate;
};

Texture2D koishi : register(t0);
Texture2D yukari : register(t1);
SamplerState colorSampler : register(s0);

float4 main(float4 pos : SV_POSITION, float2 tex : TEXCOORD) : SV_TARGET{
    return koishi.Sample(colorSampler, tex) * rate + yukari.Sample(colorSampler, tex) * (1 - rate);
}
