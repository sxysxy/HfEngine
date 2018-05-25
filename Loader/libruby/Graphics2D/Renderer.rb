#encoding: utf-8

module G2D

$__sf_code__ = HFSF::Compiler.compile_code {
Program("Renderer2D") {
	Code(%{
		struct VSInput {
			float4 pos : POSITION;
			float4 data : EXDATA;
		};
		struct VSOut {
			float4 pos : SV_POSITION;
			float4 data : EXDATA;
		};
		
		cbuffer common_param : register(b0) {
			float vp_x, vp_y, vp_w, vp_h;
		}
		
		VSOut VSJustPass(VSInput vi) {
			VSOut vo = (VSOut)vi;
			return vo;
		}
		float4 PSJustPass(VSOut vo) : SV_TARGET{
			return vo.data;
		}
		float4 PSPassEllipse(VSOut vo) : SV_TARGET{
			float a = vp_w / 2;
			float b = vp_y / 2;
			float x = vo.pos.x, y = vo.pos.y;
			x -= (a + vp_x);
			y -= (b + vp_y);
			if(x * x <= a * a && y * y <= b * b)
				return vo.data;
			else return float4(0.0f, 0.0f, 0.0f, 0.0f);
		}
		
		cbuffer sprite_vsparam : register(b1) {
			float ox, oy, angle, mirror;
		};
		cbuffer sprite_psparam : register(b2) {
			float4 color_mod;
			float opacity;
			float3 sp_useless; //only for align
		}
		VSOut VSSprite(VSInput vi) {
			return VSJustPass(vi);
		}
		float4 PSSprite(VSOut vo) : SV_TARGET{
			return PSJustPass(vo);
		}
		
	})
			
	InputLayout {
		Format "POSITION", DX::R32G32B32_FLOAT
		Format "EXDATA", DX::R32G32B32A32_FLOAT
	}
			
	Resource {
		Rasterizer("RS_fill_solid") {
			set_cull_mode DX::CULL_NONE
		}
		Rasterizer("RS_fill_wireframe") {
			set_cull_mode DX::CULL_NONE
			set_fill_mode DX::FILL_WIREFRAME
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
			set_color_blend DX::BLEND_SRC_ALPHA, DX::BLEND_INV_SRC_ALPHA, DX::BLEND_OP_ADD 
			set_alpha_blend DX::BLEND_SRC_ALPHA, DX::BLEND_INV_SRC_ALPHA, DX::BLEND_OP_ADD
		}
		
		#destRGB = (srcRGB * srcA) + destRGB
		#destA = destA
		Blender("add_blend") {
			use_default
			enable true
			set_color_blend DX::BLEND_SRC_ALPHA, DX::BLEND_DEST_COLOR, DX::BLEND_OP_ADD
			set_alpha_blend DX::BLEND_SRC_ALPHA, DX::BLEND_DEST_ALPHA, DX::BLEND_OP_ADD
		}
		
		#aim to change depth_func to LESS_EQUAL
		DepthStencilState("dss") {
			set_depth_enable true
			set_depth_func DX::COMPARISON_LESS_EQUAL;
		}
		
		ConstantBuffer("common_param") {
			set_size 16
		}
		ConstantBuffer("sprite_vsparam") {
			set_size 16
		}
		ConstantBuffer("sprite_psparam") {
			set_size 32
		}
	}
			
	Section("basic_set") {
		set_vshader("VSJustPass")
		set_pshader("PSJustPass")
		set_blender("alpha_blend")
		set_rasterizer("RS_fill_solid")
		set_depth_stencil_state("dss")
		set_vs_cbuffer(0, "common_param")
		set_vs_cbuffer(1, "sprite_vsparam")
		set_ps_cbuffer(2, "sprite_psparam")
	}
	
	Section("draw_rect") {
		set_vshader("VSJustPass")
		set_pshader("PSJustPass")
		set_rasterizer("RS_fill_solid")
	}
	
	Section("draw_ellipse") {
		set_vshader("VSJustPass")
		set_pshader("PSPassEllipse")
		set_rasterizer("RS_fill_solid")
	}
	
	Section("draw_wireframe") {
		set_vshader("VSJustPass")
		set_pshader("PSJustPass")
		set_rasterizer("RS_fill_solid")
	}
	
	Section("draw_sprite") {
		set_vshader("VSSprite")
		set_pshader("PSSprite")
		set_rasterizer("RS_fill_solid")
	}
}
}#endof $__sf_code__	


	class Renderer < DX::RenderPipelineM
		DRAW_RECT = 0
		DRAW_ELLIPSE = 1
		DRAW_WIREFRAME = 2
		DRAW_SPRITE = 3
	
		def initialize(graphics)
			super(graphics.device)
			
			@sf = HFSF::loadsf(device, $__sf_code__)[0]
			@sf.section[:basic_set].apply(self)
			@sf.input_layout.apply(self)
			
			set_target(graphics.rtt)
			set_topology(DX::TOPOLOGY_TRIANGLESTRIP)
			
			@phase = -1
			@__draw_rect = @sf.section[:draw_rect]
			@__draw_ellipse = @sf.section[:draw_ellipse]
			@__draw_wireframe = @sf.section[:draw_wireframe]
			@__draw_sprite = @sf.section[:draw_sprite]
		end
		
		#set them private, in order to avoid called from outside by users directly  
		private :set_viewport, :set_topology, :set_vshader
		
		#preparations for drawing
		def pre_rect
			if @phase != DRAW_RECT
				@phase = DRAW_RECT
				@__draw_rect.apply(self)
			end
		end
		def pre_ellipse 
			if @phase != DRAW_ELLIPSE
				@phase = DRAW_ELLIPSE
				@__draw_ellipse.apply(self)
			end
		end
		def pre_wireframe
			if @phase != DRAW_WIREFRAME
				@phase = DRAW_WIREFRAME
				@__draw_wireframe.apply(self)
			end
		end
		def pre_sprite
			if @phase != DRAW_SPRITE
				@phase = DRAW_SPRITE
				@__draw_sprite.apply(self)
			end
		end
		
		#draw 
		def draw_rect(rect, z, color)
			
		end
		
		def draw_ellipse(x, y, a, b, z, color)
			@sf.section[:draw_ellipse].apply(self)
			set_viewport(HFRect(x-a, y-b, a*2, b*2))
		end
		
		def draw_sprite(s)
			
		end
		
		alias render swap_commands #you know
		
	end

end