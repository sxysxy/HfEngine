Program("DrawTexture") {
p = <<EMMMM
	float4 VS(float4 pos : POSITION) : SV_POSITION {
		return pos;
	}
	
	cbuffer param : register(b0) {
		float4 color;
	}
	float4 PS(float4 pos : POSITION) : SV_TARGET {
		return color;
	}
EMMMM
	Code p

	Resource("res") {
		Sampler("color_sampler") {
			set_filter DX::FILTER_MIN_MAG_MIP_LINEAR, 0
			set_uvwaddress DX::ADDRESS_WRAP, DX::ADDRESS_WRAP, DX::ADDRESS_WRAP, HFColorRGBA(0.0, 0.0, 0.0, 0.0)
		}
		ConstantBuffer("param") {
			set_size 16
		}
	}
	Section("main") {
		set_ps_cbuffer("param")
		set_vshader("VS")
		set_pshader("PS")
	}
}