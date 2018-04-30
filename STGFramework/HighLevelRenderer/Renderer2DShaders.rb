Program("Draw") {
	Code(%{
		cbuffer viewport : register(b0) {
			float vp_x, vp_y, vp_w, vp_h;
		};
		struct VSDrawIn {
			float4 position : POSITION;
			float4 color : COLOR;
		};
		struct VSDrawOut {
			float4 position : SV_POSITION;
			float4 color : COLOR;
		};
		VSDrawOut VSDraw(VSDrawIn ipt) {
			VSDrawOut opt;
			opt.position.x = ipt.position.x * 2.0 / vp_w - 1.0f;
			opt.position.y = -(ipt.position.y * 2.0 / vp_h) + 1.0f;
			opt.position.z = ipt.position.z;
			opt.position.w = 1.0f;
			opt.color = ipt.color;
			return opt;
		}
		float4 PS_Color(VSDrawOut data) : SV_TARGET {
			return data.color;
		}
	})
	InputLayout {
		Format "POSITION", DX::R32G32B32_FLOAT
		Format "COLOR", DX::R32G32B32A32_FLOAT
	}
	Resource {
		Rasterizer("RS_fill_solid") {
			use_default
			set_cull_mode DX::CULL_NONE
		}
		Rasterizer("RS_fill_wireframe") {
			use_default
			set_cull_mode DX::CULL_NONE
			set_fill_mode DX::FILL_WIREFRAME
		}
		ConstantBuffer("viewport") {
			set_size 16
		}
	}
	Section("basic") {
		set_vshader("VSDraw")
		set_vs_cbuffer(0, "viewport")
		set_blender("blender")
	}
	Section("draw_solid") {
		set_vshader("VSDraw")
		set_pshader("PS_Color")
		#set_ps_sampler(0, nil)
		set_rasterizer("RS_fill_solid")
	}
	Section("draw_wireframe") {
		set_vshader("VSDraw")
		set_pshader("PS_Color")
		#set_ps_sampler(0, nil)
		set_rasterizer("RS_fill_wireframe")
	}

}

Program("Texture2D") {
	Code(%{
		cbuffer viewport : register(b0) {
			float vp_x, vp_y, vp_w, vp_h;
		}
		cbuffer angle_transform : register(b1) {
			float angle;
			int vmirror, hmirror;			//Vertical Mirror & Horizon Mirror
			int emm; //unused
		}
		struct VSTextureIn {
			float4 position : POSITION;
			float2 tex : TEXCOORD;
		};
		struct VSTextureOut {
			float4 position : SV_POSITION;
			float2 tex : TEXCOORD;
		};
		VSTextureOut VSTexture(VSTextureIn ipt) {
			VSTextureOut opt;
			float x1 = ipt.position.x * 2.0 / vp_w - 1.0f;
			float y1 = -(ipt.position.y * 2.0 / vp_h) + 1.0f;
			opt.position.x = x1 * cos(angle) - y1 * sin(angle);
			opt.position.y = x1 * sin(angle) + y1 * cos(angle);
			opt.position.z = ipt.position.z;
			opt.position.w = 1.0f;
			opt.tex = ipt.tex;
			return opt;
		}
		SamplerState color_sampler : register(s0);
		Texture2D color_map : register(t0);
		float4 PS_Texture(VSTextureOut data) : SV_TARGET {
			return color_map.Sample(color_sampler, data.tex);
		}
	})
	
	InputLayout {
		Format "POSITION", DX::R32G32B32_FLOAT
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
		ConstantBuffer("angle_transform") {
			set_size 16
			set_init_data [0.0, 0, 0, 0].pack("fi*")
		}
		ConstantBuffer("viewport") {
			set_size 16
		}
	}
	Section("basic") {
		set_vshader("VSTexture")
		set_vs_cbuffer(0, "viewport")
		set_vs_cbuffer(1, "angle_transform")
		set_blender("blender")
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

