require_relative '../CommonScene/SceneSTG.rb'
require_relative './SceneSHWStage.rb'

class SceneSHWTitle < SceneSTG
	def start
		@renderer = Renderer.new
		Graphics.re.insert(@renderer, 100)
		
		#handlers
		set_handler(:cancel, ->{SceneManager.exit})
		set_handler(:confirm, method(:on_confirm))
		
		#res
		@s_back = Sprite.new(TextureCache.load("title/back.png"))
		@s_option_bar = Sprite.new(TextureCache.load("title/selector.png"))
		@s_circle = Sprite.new(TextureCache.load("title/star.png"))
		@s_circle.x = 450
		@s_circle.y = 80
		@s_circle.z = 0.5
		@s_circle.origin_center

		@option_index = 0

		SoundManager.play_bgm("上海アリス幻樂団 - 死霊の夜桜.mp3")
	end
	
	def terminate
		Graphics.re.clear
		@renderer.release
	end
	
	def update
		super

		@renderer.clear(HFColorRGBA(0.0, 0.0, 0.0, 0.0))
		
		@renderer.draw_sprite(@s_back)
		@s_option_bar.y = 250 + @option_index * 125
		@renderer.draw_sprite(@s_option_bar)
		
		@s_circle.angle += 1
		@renderer.draw_sprite(@s_circle)
		@renderer.render
	end
	
	def update_keypress
		super
		if Controller.keyboard.is_triggled(DX::DIK_UP)
			@option_index = (@option_index+1)%2
			SoundManager.play_se("TitleCursor.ogg")
		elsif Controller.keyboard.is_triggled(DX::DIK_DOWN)
			@option_index = (@option_index+1)%2
			SoundManager.play_se("TitleCursor.ogg")
		end
		if @option_index == 0 
			@s_option_bar.x = 380
			@s_option_bar.scale_x = 1.5
		else
			@s_option_bar.x = 415
			@s_option_bar.scale_x = 0.95
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