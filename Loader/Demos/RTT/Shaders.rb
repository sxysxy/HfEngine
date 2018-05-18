Program("emm") {
	Code(%{
		struct VSInput {
			float4 pos : POSITION;
			float4 color : COLOR;
		};
		struct VSOutput {
			float4 pos  : SV_POSITION;
			float4 color : COLOR;
		};
		VSOutput VS(VSInput vi) {
			VSOutput vo = (VSOutput)vi;
			return vo;
		}
		float4 PS(VSOutput o) : SV_TARGET {
			return o.color;
		}
		
		Texture2D tex : register(t0);
		SamplerState c_sampler : register(s0);
		float4 PS2(VSOutput o) : SV_TARGET {
			return tex.Sample(c_sampler, o.color.xy);
		}
	})
	Resource {
		Rasterizer("rs") {
			set_cull_mode DX::CULL_NONE
		}
		Sampler("sp") {
			use_default
		}
	}
	InputLayout {
		Format "POSITION", DX::R32G32_FLOAT
		Format "COLOR", DX::R32G32B32A32_FLOAT
	}
	Section("set") {
		set_rasterizer("rs")
		set_vshader("VS")
		set_pshader("PS")
		set_ps_sampler(0, "sp")
	}
	Section("draw_shape") {
		set_pshader("PS")
	}
	Section("draw_texture") {
		set_pshader("PS2")
	}
}