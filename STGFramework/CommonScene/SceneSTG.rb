#encoding :utf-8
require_relative '../SceneManager.rb'
require_relative '../TextureCache.rb'

class DX::Input::Keyboard
	def is_triggled(key)
		return is_pressed_now(key) && !is_pressed_before(key)
	end
end

class SceneSTG < Scene
	def initialize
		super
		@handler = {}
	end
	
	def call_handler(h)
		@handler[h].call if @handler[h]
	end
	
	def update
		update_keypress
	end
	
	def update_keypress
		call_handler(:cancel) if @keyboard.is_triggled(DX::DIK_X)
		call_handler(:confirm) if @keyboard.is_triggled(DX::DIK_Z)
	end
end