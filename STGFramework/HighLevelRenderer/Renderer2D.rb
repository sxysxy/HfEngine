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
		
		@@__sf__ ||= HFSF.loadsf_file($device, SHADERS_FILE)[0]
		@sf = @@__sf__.copy($device)
		@quality = quality
		@sf.section[:basic].apply(self)
		@sf.input_layout.apply(self)
		set_target(Graphics.rtt)
		set_viewport(HFRect(0, 0, $window.width, $window.height))
		
		@phase = PHASE_DRAW_SOLID #phase 
	end
	
	def pre_draw_solid
		@sf.section[:draw_solid].apply(self) if @phase != PHASE_DRAW_SOLID
	end
	def pre_draw_wireframe
		@sf.section[:draw_wireframe].apply(self) if @phase != PHASE_DRAW_WIREFRAME
	end
	def pre_draw_texture
		return if @phase == PHASE_DRAW_TEXTURE
		if @quality == 1 
			@sf.section[:texturingHQ].apply(self)
		else
			@sf.section[:texturingLQ].apply(self)
		end
	end
	
	#draw rect
	def draw_rect(rect, color)
		pre_draw_solid
=begin
		    int midx = viewport.w / 2;
    int midy = viewport.h / 2;
#pragma warning(push)
#pragma warning(disable:4244)
    float x1 = 1.0 * (rect.x - midx) / midx;
    float x2 = 1.0 * (rect.x + rect.w - midx) / midx;
    float y2 = -1.0 * (rect.y - midy) / midy;
    float y1 = -1.0 * (rect.y + rect.h - midy) / midy;
#pragma warning(pop)
    struct VertexXXX {
        float pos[3];
        float tex[2];
    }vecs[] = {
        {{ x1, y1, _z_depth },{ 0.0, 1.0 }},
    {{ x1, y2, _z_depth },{ 0.0, 0.0 }},
    {{ x2, y1, _z_depth },{ 1.0, 1.0 }},
    {{ x2, y2, _z_depth },{ 1.0, 0.0 }}
    };
=end
		midx = viewport.w / 2
		midy = viewport.h / 2
		x1 = 1.0 * (rect.x - midx) / midx
		x2 = 1.0 * (rect.x + rect.w - midx) / midx;
		y2 = -1.0 * (rect.y - midy) / midy
		y1 = -1.0 * (rect.y + rect.h - midy) / midy
		vecs = [[x1, y2, 0], [color.r, color.g, color.b, color.a],
				[x1, y2, 0], [color.r, color.g, color.b, color.a],
				[x1, y2, 0], [color.r, color.g, color.b, color.a],
				[x1, y2, 0], [color.r, color.g, color.b, color.a]].flatten.pack("f*")
		vb = DX::VertexBuffer.new($device, 7*4, 4, vecs)
		set_topology(DX::TOPOLOGY_TRIANGLESTRIP)
		set_vbuffer(vb)
		draw(0, 4)
		vb.release
	end
	
	def render
		Graphics.re.push(self)
	end
	
	def self.release_basic_resource
		@@__sf__.release if @@__sf__
	end
end