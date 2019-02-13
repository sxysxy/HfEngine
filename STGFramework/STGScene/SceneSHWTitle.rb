require_relative '../CommonScene/SceneTitle.rb'
require_relative './SceneSHWStage.rb'

class SceneSHWTitle < SceneTitle
	def start
		@renderer = Renderer.new
		Graphics.re.insert(@renderer, 100)
		
		#handlers
		set_handler(:cancel, ->{SceneManager.exit})
		set_handler(:confirm, method(:on_confirm))
		
		#res
		@s_back = Sprite.new(TextureCache.load("title/back.png"))
		@s_option_bar = Sprite.new(TextureCache.load("title/cursor.png"))
		@s_option_bar.opacity = 0.5
		@s_circle = Sprite.new(TextureCache.load("title/circle.png"))
		@s_circle.x = 410
		@s_circle.y = 330
		@s_circle.origin_center

		@option_index = 0
	end
	
	def terminate
		Graphics.re.clear
		@renderer.release
	end
	
	def update
		super
		@renderer.clear(HFColorRGBA(0.0, 0.0, 0.0, 0.0))
		
		@renderer.draw_sprite(@s_back)
		@s_option_bar.x = 380
		@s_option_bar.y = 170 + @option_index * 80
		@renderer.draw_sprite(@s_option_bar)
		
		@s_circle.angle += 3
		@renderer.draw_sprite(@s_circle)
		@renderer.render
	end
	
	def update_keypress
		super
		if Controller.keyboard.is_triggled(DX::DIK_UP)
			@option_index = (@option_index+1)%2
		elsif Controller.keyboard.is_triggled(DX::DIK_DOWN)
			@option_index = (@option_index+1)%2
		end
	end
	
	def on_confirm
		if @option_index == 1
			SceneManager.exit
		elsif @option_index == 0
			SceneManager.call(SceneSHWStage)
		end
	end
	
end