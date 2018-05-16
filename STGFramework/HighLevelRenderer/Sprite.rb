require_relative "./Graphics2D.rb"

class Sprite
	attr_reader :texture
	attr_accessor :src_rect
	attr_accessor :angle
	attr_accessor :opacity
	attr_accessor :z
	attr_accessor :dest_rect
	attr_accessor :color_mod
	attr_accessor :viewport
	attr_accessor :ox, :oy
	attr_accessor :hmirror, :vmirror
	
	def initialize(t)
		self.texture = t
	end
	
	def texture=(t)
		@texture = t
		@src_rect = HFRect(0, 0, t.width, t.height)
		@angle = 0
		@opacity = 1.0
		@dest_rect = HFRect(0, 0, t.width, t.height)
		@z = 0.0
		@color_mod = HFColorRGBA(1.0, 1.0, 1.0, 1.0)
		@viewport = HFRect(0, 0, Graphics.width, Graphics.height)
		@hmirror = @vmirror = false
		@ox = @oy = 0
	end
	
	def x=(x_)
		@dest_rect.x = x_
	end
	def y=(y_)
		@dest_rect.y = y_
	end
	def x
		@dest_rect.x
	end
	def y
		@dest_rect.y
	end
	def zoom_x=(zx) 
		@dest_rect.width = @texture.width * zx
	end
	def zoom_y=(zy)
		@dest_rect.height = @texture.height * zy;
	end
	def origin_LT
		@ox = @oy = 0
	end
	def origin_center
		@ox = @texture.width / 2
		@oy = @texture.height / 2
	end
	
	def release_texture
		@texture.release
	end
	
	alias :set_texture :texture=
end