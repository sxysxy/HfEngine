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
		Blender("blender") {
			enable true
								  #注意 &不要和~连起来写...
			set_mask DX::COLOR_WRITE_ENABLE_ALL & ~DX::COLOR_WRITE_ENABLE_GREEN  #清除原谅色~
			set_color_blend DX::BLEND_FACTOR, DX::BLEND_ONE, DX::BLEND_OP_ADD
			set_blend_factor HFColorRGBA(0.5, 0.0, 0.8, 1.0)
		}
	}
	Section("main") {
		set_ps_cbuffer("param")
		set_vshader("VS")
		set_pshader("PS")
	}
}