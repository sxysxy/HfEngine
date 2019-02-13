#encoding: utf-8
module G2D
	class Sprite
		attr_reader :texture
		attr_accessor :ox, :oy, :z
		attr_accessor :src_rect, :dest_rect
		attr_accessor :angle, :mirror
		attr_accessor :color_mod, :viewport
		attr_accessor :opacity
		
		def initialize(t = nil)
			@ox = @oy = @angle = 0
			@z = 1.0
			@mirror = false
			@opacity = 1.0
			@color_mod = HFColorRGBA(1.0, 1.0, 1.0, 1.0)
			@viewport = HFRect(0, 0, G2D::Graphics.width, G2D::Graphics.height)
			
			self.texture=(t)
		end
		
		def texture=(t)
			@texture = t
			if(t)
				@src_rect = HFRect(0, 0, t.width, t.height)
				@dest_rect = @src_rect.clone
			end
		end
		
		#operators to the dest_rect
		def scale_x=(sx)
			@dest_rect.width = @texture.width * sx
		end
		def scale_y=(sy)
			@dest_rect.height = @texture.height * sy
		end
		def x=(nx)
			@dest_rect.x = nx
		end
		def y=(ny)
			@dest_rect.y = ny
		end
		def width
			@dest_rect.width
		end
		def height
			@dest_rect.height
		end
		def x
			@dest_rect.x
		end
		def y
			@dest_rect.y
		end
		
		def origin_center
			@ox = @dest_rect.width / 2
			@oy = @dest_rect.height / 2
		end
		def release_texture
			@texture.release if @texture.is_a?(DX::Texture2D)
		end
	end
end