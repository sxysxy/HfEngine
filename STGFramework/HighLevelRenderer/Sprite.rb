require_relative "./Graphics2D.rb"

class Sprite
	attr_reader :texture
	attr_accessor :src_rect
	attr_accessor :angle
	attr_accessor :opacity
	attr_accessor :x, :y, :z, :zoom_x, :zoom_y
	attr_accessor :color_mod
	attr_accessor :viewport
	attr_accessor :hmirror, :vmirror
	
	def initialize(t)
		self.texture = t
	end
	
	def texture=(t)
		@texture = t
		@src_rect = HFRect(0, 0, t.width, t.height)
		@angle = 0
		@opacity = 1.0
		@x = @y = 0
		@z = 0.0
		@zoom_x = @zoom_y = 1.0
		@color_mod = HFColorRGBA(1.0, 1.0, 1.0, 1.0)
		@viewport = HFRect(0, 0, Graphics.width, Graphics.height)
		@hmirror = @vmirror = false
	end
	
	alias :set_texture :texture=
end