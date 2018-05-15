class STGObject 
	attr_reader :frame_time
	
	attr_accessor :on_create, :on_logic_update, :on_render_update
	
	def initialize
		@frame_time = 0
	end
	
	def on_create_m
		instance_exec &on_create
	end
	
	def on_logic_update_m
		@frame_time += 1
		instance_exec &on_logic_update
	end
	
	def on_render_update_m
		instance_exec &on_render_update
	end
	
end