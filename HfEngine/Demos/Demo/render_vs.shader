struct vs_output {
	float4 position : SV_POSITION;
	float4 color : COLOR;
};

struct vs_input {
	float4 position : POSITION;
	float4 color : COLOR;
};

vs_output main(vs_input input) {
	vs_output opt;
	opt.position = input.position;
	opt.color = input.color;
	return opt;
}



