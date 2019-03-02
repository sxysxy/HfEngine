=begin
	STG Game Objects. Including Player, Bullet, Enemy, etc.
=end

class STGObject
	attr_reader :frame_time    #frame time since created
	attr_accessor :on_create, :on_logic_update, :on_render_update  #callback functions
	attr_accessor :sprite
	attr_accessor :logical_x, :logical_y

	def initialize(_sprite = nil)
		@frame_time = 0
		@sprite = _sprite

		@on_create = -> {}
		@on_logic_update = -> {}
		@on_render_update = ->{|renderer|
			@sprite.x = @logical_x
			@sprite.y = @logical_y
			if @sprite
				renderer.draw_sprite(@sprite)
			end
		}
		@logical_x = @logical_y = 0
	end
	
	def exec_on_create
		instance_exec &on_create
	end
	
	def exec_logic_update
		@frame_time += 1
		instance_exec &on_logic_update
	end
	
	def exec_render_update
		instance_exec &on_render_update
	end
	
end

class Player < STGObject 
	def initialize(image)
		super
		@sprite.texture = image
	end
end

class Enemy < STGObject
	def initialize(image, x, y)
		super
		@sprite.texture = image
	end
end