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
		
		cbuffer VSTextureParam : register(b1) {
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
		cbuffer PSTextureParam : register(b2) {
			float4 color_mod;
			float opacity;
			float psp_em, psp_emm, psp_emmm;
		}
		SamplerState color_sampler : register(s0);
		Texture2D color_map : register(t0);
		float4 PSTexture(VSOut data) : SV_TARGET {
			float4 c = color_map.Sample(color_sampler, data.tex);
			c.a *= clamp(opacity, 0.0, 1.0);
			c *= color_mod;
			return c;
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
		ConstantBuffer("VSTextureParam") {
			set_size 16
			set_init_data [0.0, 0, 0, 0].pack("fi*")
		}
		ConstantBuffer("PSTextureParam") {
			set_size 32
			set_init_data [1.0, 1.0, 1.0, 1.0, 1.0, 0.0, 0.0, 0.0].pack("f*")
		}
		
		#destRGBA = srcRGBA
		#(dest is the color in the renderring-target texture)
		#(src is the color in resoruce)
		Blender("no_blend") {
			use_default
			enable true
		}
		
		#destRGB = (srcRGB * srcA) + destRGB * (1-srcA))
		#destA = srcA + (destA * (1-srcA))
		Blender("alpha_blend") {
			use_default
			enable true
			set_color_blend DX::BLEND_ONE, DX::BLEND_FACTOR, DX::BLEND_OP_ADD 
			set_alpha_blend DX::BLEND_ONE, DX::BLEND_ONE, DX::BLEND_OP_ADD
		}
		
		#destRGB = (srcRGB * srcA) + destRGB
		#destA = destA
		Blender("add_blend") {
		
		}
		
		#add -> sub
		#Blender("sub_blend") {
		#}
		
		#color modulation
		#destRGB = srcRGB * destRGB
		#destA = destA
		Blender("color_mod_blender") {
			
		}
	}
	
	Section("basic") {
		set_vshader("VSDraw")
		set_vs_cbuffer(0, "viewport")
		set_vs_cbuffer(1, "VSTextureParam")
		set_ps_cbuffer(2, "PSTextureParam")
		set_blender("alpha_blend")
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
		set_rasterizer("RS_fill_solid")
	}
	Section("texturingLQ") {
		set_vshader("VSTexture")
		set_pshader("PSTexture")
		set_ps_sampler(0, "LQSampler")
		set_rasterizer("RS_fill_solid")
	}
	
}



