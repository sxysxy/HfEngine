#encoding: utf-8

module G2D

#HSSF DSL
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
		float4 PSJustPass(VSOut vo) : SV_TARGET {
			return vo.data;
		}
		float4 PSPassEllipse(VSOut vo) : SV_TARGET {
			float a = vp_w / 2;
			float b = vp_h / 2;
			float x = vo.pos.x, y = vo.pos.y;
			x -= (a + vp_x);
			y -= (b + vp_y);
			if(x * x / a * a + y * y / b * b <= 1.0f)
				return vo.data;
			else return float4(0.0f, 0.0f, 0.0f, 0.0f);	
		}
		
		cbuffer sprite_vsparam : register(b1) {
			float angle, ox, oy;
			float svs_useless;
		};
		cbuffer sprite_psparam : register(b2) {
			float4 color_mod;
			float opacity;
			float3 sp_useless; //only for align
		}
		VSOut VSSprite(VSInput vi) {
			VSOut opt;
			
			float x1 = (vi.pos.x + vp_x) * 2.0 / vp_w - 1.0f;
			float y1 = -((vi.pos.y + vp_y) * 2.0 / vp_h) + 1.0f;
			
			float x2 = x1 * cos(angle) - y1 * sin(angle);
			float y2 = x1 * sin(angle) + y1 * cos(angle);
			
			opt.pos.x = x2;
			opt.pos.y = y2;
			opt.pos.z = vi.pos.z;
			opt.pos.w = 1.0f;
			opt.data = vi.data;
			return opt;
		}
		
		SamplerState color_sampler : register(s0);
		Texture2D color_map : register(t0);
		float4 PSSprite(VSOut vo) : SV_TARGET{
			float4 c = color_map.Sample(color_sampler, vo.data.xy);
			c.rgb *= color_mod.rgb;
			c.a *= opacity;
			return c;
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
		
		Sampler("sampler") {
			use_default
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
		set_ps_sampler(0, "sampler")
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
	
		def initialize(graphics, priority = 100)
			super(graphics.device)
			@graphics = graphics
			@re = graphics.render_exec
			
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
			@__cb_common_param = @sf.resource.cbuffer[:common_param]
			@__cb_sprite_vsparam = @sf.resource.cbuffer[:sprite_vsparam]
			@__cb_sprite_psparam = @sf.resource.cbuffer[:sprite_psparam]
			
			#make a vertex buffer which covers an area of rectangle.
			@__vb_rect = DX::VertexBuffer.new(@device, 7*4, 4)
			set_vbuffer(@__vb_rect)
			
			@re.lock
			@re.insert(self, priority)
			@re.unlock
		end
		
		def set_viewport(r)
			super(r)
			update_subresource @__cb_common_param, [r.x, r.y, r.w, r.h].pack("f*")
		end
		
		#set them private, in order to avoid called from outside by users directly  
		private :set_viewport, :set_topology, :set_vshader, :set_rasterizer, :draw, :draw_index
		
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
		def mark_dirty
			@phase -1
		end
		
		#draw 
		def draw_rect(rect, z, color)
			pre_rect
			set_viewport(HFRect(rect))
			
			vecs = [-1.0, +1.0, z, color.r, color.g, color.b, color.a,
					+1.0, +1.0, z, color.r, color.g, color.b, color.a,
					-1.0, -1.0, z, color.r, color.g, color.b, color.a,
					+1.0, -1.0, z, color.r, color.g, color.b, color.a].pack("f*")
			update_subresource @__vb_rect, vecs
			draw(0, 4)
		end
		
		def draw_ellipse(x, y, a, b, z, color)
			pre_ellipse
			set_viewport(HFRect(x-a, y-b, a*2, b*2))
			
			vecs = [-1.0, +1.0, z, color.r, color.g, color.b, color.a,
					+1.0, +1.0, z, color.r, color.g, color.b, color.a,
					-1.0, -1.0, z, color.r, color.g, color.b, color.a,
					+1.0, -1.0, z, color.r, color.g, color.b, color.a].pack("f*")
			update_subresource @__vb_rect, vecs
			draw(0, 4)
		end
		
		def draw_circle(x, y, r, color)
			draw_ellipse(x, y, r, r, color)
		end
		
		def draw_sprite(s)
			pre_sprite
			set_viewport(s.viewport)
			
			rect = s.src_rect
			w = s.texture.width
			h = s.texture.height
			#LT
			x1 = Float(rect.x) / w  
			y1 = Float(rect.y) / h 
			#RT
			x2 = Float(rect.x + rect.w) / w
			y2 = y1
			#LB
			x3 = x1
			y3 = Float(rect.y + rect.h) / h
			#RB
			x4 = x2
			y4 = y3
			
			dest_rect = s.dest_rect
			dx = Float(dest_rect.x - s.ox)
			dy = Float(dest_rect.y - s.oy)
			
			set_ps_resource(0, s.texture)
			vecs = [dx, dy, 					    s.z, x1, y1, 0.0, 0.0,
					dx+dest_rect.w, dy, 		    s.z, x2, y2, 0.0, 0.0,
					dx, dy+dest_rect.h, 		    s.z, x3, y3, 0.0, 0.0,
					dx+dest_rect.w, dy+dest_rect.h, s.z, x4, y4, 0.0, 0.0].pack("f*")
			update_subresource @__vb_rect, vecs
			update_subresource @__cb_sprite_psparam, [s.color_mod.r, s.color_mod.g, s.color_mod.b, s.color_mod.a,
													 s.opacity, 0.0, 0.0, 0.0].pack("f*")
			update_subresource @__cb_sprite_vsparam, [s.angle, s.ox, s.oy, 0.0].pack("f*")
			draw(0, 4)
			set_ps_resource(0, nil)
		end
		
		def use_default_target
			set_target @graphics.rtt
		end
		
		def clear(c = HFColorRGBA(0.0, 0.0, 0.0, 0.0))
			super(c)
		end
		
		alias render swap_commands #you know
		
		def release
			[@sf, @__vb_rect].each &:release
			@re.erase(self)
			super
		end
		
	end

end