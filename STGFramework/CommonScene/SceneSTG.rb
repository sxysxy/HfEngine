#encoding :utf-8
=begin
	STG Scene Basic

=end
require_relative '../SceneManager.rb'
require_relative '../TextureCache.rb'

class DX::Input::Keyboard
	def is_triggled(key) #judge triggled
		return is_pressed_now(key) && !is_pressed_before(key)
	end
end

class SceneSTG < Scene
	def initialize
		super 
		reset 
	end
	
	#reset some attribute
	def reset
		@handler = {}
	end
	
	#call a handler by symbol h
	def call_handler(h)
		@handler[h].call if @handler[h]
	end
	
	#set a handler named symbol h
	def set_handler(h, callback)
		@handler[h] = callback
	end
	
	#update every frame(in principle)
	def update
		update_keypress
	end
	
	
	def update_keypress
		call_handler(:cancel) if @keyboard.is_triggled(DX::DIK_X) 
		call_handler(:confirm) if @keyboard.is_triggled(DX::DIK_Z) || @keyboard.is_triggled(DX::DIK_RETURN)
	end
end