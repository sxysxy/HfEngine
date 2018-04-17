//VS
cbuffer wvpmatrix : register(b0) {
	float4x4 wvp;
};

struct vs_output{
	float4 pos : SV_POSITION;
	float4 color : COLOR;
};

vs_output VS(float4 pos : POSITION, float4 color : COLOR) {
	vs_output opt;
	opt.pos = mul(pos, wvp);
	opt.color = color;
	return opt;
}

float4 PS(vs_output v) : SV_TARGET {
	return v.color;
}


