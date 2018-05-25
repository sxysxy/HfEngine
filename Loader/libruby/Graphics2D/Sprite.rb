#encoding: utf-8
module G2D
	class Sprite
		attr_reader :texture
		attr_accessor :ox, :oy, :z
		attr_accessor :src_rect, :dest_rect
		attr_accessor :angle, :mirror
		attr_accessor :color_mod, :viewport
		attr_accessor :opacity
		
		def initialize(graphics, t = nil)
			@ox = @oy = @angle = 0
			@z = 0.0
			@mirror = false
			@opacity = 1.0
			@color_mod = HFColorRGBA(1.0, 1.0, 1.0, 1.0)
			@viewport = HFRect(0, 0, graphics.width, graphics.height)
			
			self.texture=(t)
		end
		
		def texture=(t)
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
	end
end