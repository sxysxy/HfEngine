#encoding :utf-8
require_relative '../SceneManager.rb'
require_relative '../TextureCache.rb'

class SceneTitle < Scene
	def start
		
	end
	
	def terminate
	
	end
	
	def update
		SceneManager.exit if @keyboard.is_pressed_now(DX::DIK_ESCAPE)
	end
end