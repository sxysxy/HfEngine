#encoding :utf-8
=begin
	Renderer2D
	
=end

require_relative "./Graphics2D.rb"

class Renderer2D < DX::RenderPipeline
	SHADERS_FILE = "./HighLevelRenderer/Renderer2DShaders.rb"

	PHASE_DRAW_SOLID = 0
	PHASE_DRAW_WIREFRAME = 1
	PHASE_DRAW_TEXTURE = 2
	
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
		
		@phase = PHASE_DRAW_SOLID - 1 #phase 
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
		vecs = [[Float(rect.x), Float(rect.y), 0], [color.r, color.g, color.b, color.a],
				[Float(rect.x+rect.w), Float(rect.y), 0], [color.r, color.g, color.b, color.a],
				[Float(rect.x), Float(rect.y+rect.h), 0], [color.r, color.g, color.b, color.a],
				[Float(rect.x+rect.w), Float(rect.y+rect.h), 0], [color.r, color.g, color.b, color.a]].flatten.pack("f*")
		vb = DX::VertexBuffer.new($device, 7*4, 4, vecs)
		set_topology(DX::TOPOLOGY_TRIANGLESTRIP)
		set_vbuffer(vb)
		draw(0, 4)
		vb.release
	end
	
	def draw_texture(texture, dest_rect, src_rect = nil)
		pre_draw_texture
		
		rect = src_rect ? src_rect : HFRect(0, 0, texture.width, texture.height)
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
		
		vecs = [[Float(dest_rect.x), Float(dest_rect.y),             0], [x1, y1],
				[Float(dest_rect.x+dest_rect.w), Float(dest_rect.y), 0], [x2, y2],
				[Float(dest_rect.x), Float(dest_rect.x+dest_rect.h), 0], [x3, y3],
				[Float(dest_rect.x+dest_rect.w), Float(dest_rect.y+dest_rect.h), 0],[x4, y4]].flatten.pack("f*")
		vb = DX::VertexBuffer.new($device, 5*4, 4, vecs)
		set_topology(DX::TOPOLOGY_TRIANGLESTRIP)
		set_vbuffer(vb)
		draw(0, 4)
		vb.release
	end
	
	def render
		Graphics.re.push(self)
	end
	
	
	def release
		super
		@sf.release
	end
end