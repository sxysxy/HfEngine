#encoding :utf-8
=begin
	Renderer2D
	
=end

require_relative "./Graphics2D.rb"

class Renderer2D < DX::RenderPipelineM
	SHADERS_FILE = "./HighLevelRenderer/Renderer2DShaders.rb"

	PHASE_DRAW_SOLID = 0
	PHASE_DRAW_WIREFRAME = 1
	PHASE_DRAW_TEXTURE = 2
	PHASE_DRAW_SPRITE = 3
	
	attr_accessor :z_depth #z depth
	
	#quality = 1, high quality
	#quality = 0, low quality
	def initialize(quality = 1)
		super($device)
	
		@sf = HFSF.loadsf_file($device, SHADERS_FILE)[0]
		@quality = quality
		
		@sf.section[:basic].apply(self)
		@sf.input_layout.apply(self)
		set_target(Graphics.rtt)
		set_viewport(HFRect(0, 0, $window.width, $window.height))
		Graphics.re.lock
		immdiate_render
		Graphics.re.unlock
		
		@z_depth = 0.0;
		@phase = PHASE_DRAW_SOLID - 1 #phase 
		
		#common vertex buffer for drawing rect or texturing
		@common_vbuffer = DX::VertexBuffer.new($device, 9*4, 4)
	end
	
	def pre_draw_solid
		@sf.section[:draw_solid].apply(self) if @phase != PHASE_DRAW_SOLID
		@phase = PHASE_DRAW_SOLID
	end
	def pre_draw_wireframe
		@sf.section[:draw_wireframe].apply(self) if @phase != PHASE_DRAW_WIREFRAME
		@phase = PHASE_DRAW_WIREFRAME
	end
	def pre_draw_texture
		return if @phase == PHASE_DRAW_TEXTURE
		if @quality == 1 
			@sf.section[:texturingHQ].apply(self)
		else
			@sf.section[:texturingLQ].apply(self)
		end
		@phase = PHASE_DRAW_TEXTURE
	end
	def pre_draw_sprite
		return if @phase == PHASE_DRAW_SPRITE
		if @quality == 1
			@sf.section[:SpriteHQ].apply(self)
		else 
			@sf.section[:SpriteLQ].apply(self)
		end
		@phase = PHASE_DRAW_SPRITE
	end
	
	def set_viewport(r)
		super(r)
		update_subresource @sf.resource.cbuffer[:viewport], [viewport.x, viewport.y, viewport.w, viewport.h].pack("f*")
	end
	
	#draw rect
	def draw_rect(rect, color)
		pre_draw_solid
		#LT
		#x1 = (rect.x * 2.0) / viewport.w
		#y1 = (rect.y * 2.0) / viewport.h
		#RT
		#x2 = (rect.x + rect.w) * 2.0 / viewport.w
		#y2 = rect.y * 2.0 / viewport.h
		#LB
		#x3 = (rect.x * 2.0) / viewport.w
		#y3 = (rect.y + rect.h) * 2.0 / viewport.h
		#RB
		#x4 = (rect.x + rect.w) * 2.0 / viewport.w
		#y4 = (rect.y + rect.h) * 2.0 / viewport.h
		
		#vecs = [[x1, y1, 0], [color.r, color.g, color.b, color.a],
		#		[x2, y2, 0], [color.r, color.g, color.b, color.a],
		#		[x3, y3, 0], [color.r, color.g, color.b, color.a],
		#		[x4, y4, 0], [color.r, color.g, color.b, color.a]].flatten.pack("f*")
		vecs = [Float(rect.x), Float(rect.y), @z_depth, color.r, color.g, color.b, color.a, 0.0, 0.0,
				Float(rect.x+rect.w), Float(rect.y), @z_depth, color.r, color.g, color.b, color.a, 0.0, 0.0,
				Float(rect.x), Float(rect.y+rect.h), @z_depth, color.r, color.g, color.b, color.a, 0.0, 0.0,
				Float(rect.x+rect.w), Float(rect.y+rect.h), @z_depth, color.r, color.g, color.b, color.a, 0.0, 0.0].pack("f*")
		update_subresource @common_vbuffer, vecs
		set_topology(DX::TOPOLOGY_TRIANGLESTRIP)
		set_vbuffer(@common_vbuffer)
		draw(0, 4)
	end
	
	def draw_texture(texture, dest_rect, src_rect = nil, opacity = 1.0)
		pre_draw_texture
		
		rect = src_rect ? src_rect : HFRect(0, 0, texture.width, texture.height)
		w = texture.width
		h = texture.height
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
		
		vecs = [Float(dest_rect.x), Float(dest_rect.y),             @z_depth, 0.0, 0.0, 0.0, 0.0, x1, y1, 
				Float(dest_rect.x+dest_rect.w), Float(dest_rect.y), @z_depth, 0.0, 0.0, 0.0, 0.0, x2, y2,
				Float(dest_rect.x), Float(dest_rect.y+dest_rect.h), @z_depth, 0.0, 0.0, 0.0, 0.0, x3, y3,
				Float(dest_rect.x+dest_rect.w), Float(dest_rect.y+dest_rect.h), @z_depth, 0.0, 0.0, 0.0, 0.0, x4, y4].pack("f*")
		update_subresource @common_vbuffer, vecs
		update_subresource @sf.resource.cbuffer[:PSTextureParam], [1.0, 1.0, 1.0, 1.0, opacity, 0.0, 0.0, 0.0].pack("f*")
		set_topology(DX::TOPOLOGY_TRIANGLESTRIP)
		set_vbuffer(@common_vbuffer)
		set_ps_resource(0, texture)
		draw(0, 4)
		set_ps_resource(0, nil)
	end
	
	def draw_sprite(s)
		pre_draw_sprite
		
		rect = s.src_rect
		w = rect.w
		h = rect.h
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
		
		dest_rect = HFRect.new(s.x, s.y, Integer(s.texture.width * s.zoom_x), Integer(s.texture.height * s.zoom_y))
		
		vecs = [Float(dest_rect.x), Float(dest_rect.y),             s.z, 0.0, 0.0, 0.0, 0.0, x1, y1, 
				Float(dest_rect.x+dest_rect.w), Float(dest_rect.y), s.z, 0.0, 0.0, 0.0, 0.0, x2, y2,
				Float(dest_rect.x), Float(dest_rect.y+dest_rect.h), s.z, 0.0, 0.0, 0.0, 0.0, x3, y3,
				Float(dest_rect.x+dest_rect.w), Float(dest_rect.y+dest_rect.h), s.z, 0.0, 0.0, 0.0, 0.0, x4, y4].pack("f*")
		update_subresource @common_vbuffer, vecs
		update_subresource @sf.resource.cbuffer[:PSTextureParamSprite], 
				[s.color_mod.r, s.color_mod.g, s.color_mod.b, s.color_mod.a, s.opacity, 0.0, 0.0, 0.0].pack("f*")
		update_subresource @sf.resource.cbuffer[:VSTextureEXParam], 
			[s.viewport.x, s.viewport.y, s.viewport.w, s.viewport.h, s.angle, s.vmirror ? 1 : 0, s.hmirror ? 1 : 0, 0].pack("fffffiii")			
		set_topology(DX::TOPOLOGY_TRIANGLESTRIP)
		set_vbuffer(@common_vbuffer)
		set_ps_resource(0, s.texture)
		draw(0, 4)
		set_ps_resource(0, nil)
	end
	
	def render
		swap_commands
	end
	
	def use_default_target
		set_target Graphics.rtt
	end
	
	def release
		super
		@sf.release
		@common_vbuffer.release
	end
end