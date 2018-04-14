struct vs_output {
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD;
};

vs_output VS(float4 pos : POSITION, float2 tex : TEXCOORD) {
	vs_output opt;
	opt.pos = pos;
	opt.tex = tex;
	return opt;
}

cbuffer param : register(b0) {
	float rate;
	float em, emm, emmm; //useless
};

Texture2D koishi : register(t0);
Texture2D yukari : register(t1);
SamplerState color_sampler : register(s0);

float4 PS(vs_output vpt) : SV_TARGET {
	return koishi.Sample(color_sampler, vpt.tex) * rate + yukari.Sample(color_sampler, vpt.tex) * (1 - rate);
}