Program("Draw") {
	Code(%{
		struct Input {
			float3 position : POSITION;
			float4 color : COLOR;
			float2 tex : TEXCOORD;
		};
		struct VSOut {
			float4 position : SV_POSITION;
			float4 color : COLOR;
			float2 tex : TEXCOORD;
		};
		cbuffer viewport : register(b0) {
			float vp_x, vp_y, vp_w, vp_h;
		};
		
		VSOut VSDraw(Input ipt) {
			VSOut opt;
			opt.position.x = ipt.position.x * 2.0 / vp_w - 1.0f;
			opt.position.y = -(ipt.position.y * 2.0 / vp_h) + 1.0f;
			opt.position.z = ipt.position.z;
			opt.position.w = 1.0f;
			opt.color = ipt.color;
			opt.tex = float2(0.0, 0.0);
			return opt;
		}
		float4 PSColor(VSOut data) : SV_TARGET {
			return data.color;
		}
		
		cbuffer angle_transform : register(b1) {
			float angle;
			int vmirror, hmirror;			//Vertical Mirror & Horizon Mirror
			int emm; //unused
		};
		VSOut VSTexture(Input ipt) {
			VSOut opt;
			float x1 = ipt.position.x * 2.0 / vp_w - 1.0f;
			float y1 = -(ipt.position.y * 2.0 / vp_h) + 1.0f;
			opt.position.x = x1 * cos(angle) - y1 * sin(angle);
			opt.position.y = x1 * sin(angle) + y1 * cos(angle);
			opt.position.z = ipt.position.z;
			opt.position.w = 1.0f;
			opt.tex = ipt.tex;
			opt.color = float4(0.0, 0.0, 0.0, 0.0);
			return opt;
		}
		
		SamplerState color_sampler : register(s0);
		Texture2D color_map : register(t0);
		float4 PSTexture(VSOut data) : SV_TARGET {
			return color_map.Sample(color_sampler, data.tex);
		}
	})
	InputLayout {
		Format "POSITION", DX::R32G32B32_FLOAT
		Format "COLOR", DX::R32G32B32A32_FLOAT
		Format "TEXCOORD", DX::R32G32_FLOAT
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
		Sampler("LQSampler") {
			use_default
			set_filter DX::FILTER_MIN_MAG_MIP_POINT, 0
		}
		Sampler("HQSampler") {
			use_default
		}
		ConstantBuffer("angle_transform") {
			set_size 16
			set_init_data [0.0, 0, 0, 0].pack("fi*")
		}
		Blender("blender") {
			use_default
		}
	}
	
	Section("basic") {
		set_vshader("VSDraw")
		set_vs_cbuffer(0, "viewport")
		set_vs_cbuffer(1, "angle_transform")
		set_blender("blender")
	}
	Section("draw_solid") {
		set_vshader("VSDraw")
		set_pshader("PSColor")
		set_rasterizer("RS_fill_solid")
	}
	Section("draw_wireframe") {
		set_vshader("VSDraw")
		set_pshader("PSColor")
		set_rasterizer("RS_fill_wireframe")
	}
	Section("texturingHQ") {
		set_vshader("VSTexture")
		set_pshader("PSTexture")
		set_ps_sampler(0, "HQSampler")
	}
	Section("texturingLQ") {
		set_vshader("VSTexture")
		set_pshader("PSTexture")
		set_ps_sampler(0, "LQSampler")
	}
	
}



