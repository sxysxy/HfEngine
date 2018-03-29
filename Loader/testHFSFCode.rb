Program("DrawTexture") {
p = <<EMMMM
	cbuffer param : register(b0) {
		float zoom_x, zoom_y;
	};
	float4 VS(float4 pos : POSITION) : SV_POSITION {
		return pos;
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
		set_vshader("VS")
	}
}