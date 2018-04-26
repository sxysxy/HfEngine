Program("Renderer2D") {
	Code(%{
		struct VSIn{
			float4 position : POSITION;
			float4 color : COLOR;
			float2 tex : TEXCOORD;
		};
		struct VSOut {
			float4 position : SV_POSITION;
			float4 color : COLOR;
			float2 tex : TEXCOORD;
		};
		VSOut VS(VSIn ipt) {
			VSOut opt = (VSOut)ipt;
			return opt;
		}
		SamplerState color_sampler : register(s0);
		Texture2D color_map : register(t0);
		float4 PS_Texture(VSOut data) : SV_TARGET {
			return color_map.Sample(color_sampler, data.tex);
		}
		float4 PS_Color(VSOut data) : SV_TARGET {
			return data.color;
		}
	})
	InputLayout {
		Format "POSITION", DX::R32G32B32_FLOAT
		Format "COLOR", DX::R32G32B32A32_FLOAT
		Format "TEXCOORD", DX::R32G32_FLOAT
	}
	Resource {
		Sampler("LQSampler") {
			use_default
			set_filter DX::FILTER_MIN_MAG_MIP_POINT, 0
		}
		Sampler("HQSampler") {
			use_default
		}
		Blender("blender") {
			use_default
		}
		Rasterizer("RS_fill_solid") {
			use_default
			set_cull_mode DX::CULL_NONE
		}
		Rasterizer("RS_fill_wireframe") {
			use_default
			set_cull_mode DX::CULL_NONE
			set_fill_mode DX::FILL_WIREFRAME
		}
	}
	Section("basic") {
		set_vshader("VS")
		set_blender("blender")
	}
	Section("draw_solid") {
		set_pshader("PS_Color")
		set_ps_sampler(0, nil)
		set_rasterizer("RS_fill_solid")
	}
	Section("draw_wireframe") {
		set_pshader("PS_Color")
		set_ps_sampler(0, nil)
		set_rasterizer("RS_fill_wireframe")
	}
	Section("texturingHQ") {
		set_pshader("PS_Texture")
		set_ps_sampler(0, "HQSampler")
	}
	Section("texturingLQ") {
		set_pshader("PS_Texture")
		set_ps_sampler(0, "LQSampler")
	}
}