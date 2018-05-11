require_relative '../CommonScene/SceneTitle.rb'
require_relative '../HighLevelRenderer/Renderer2D.rb'
require_relative '../HighLevelRenderer/Sprite.rb'
require_relative './SceneSHWStage.rb'

class SceneSHWTitle < SceneTitle
	
	def start
		@renderer = Renderer2D.new
		Graphics.re.insert(@renderer, 100)
		
		#handlers
		set_handler(:cancel, ->{SceneManager.exit})
		set_handler(:confirm, method(:on_confirm))
		
		#res
		@back = TextureCache.load("title/back.png")
		@option_bar = TextureCache.load("title/cursor.png")
		@s_circle = Sprite.new(TextureCache.load("title/circle.png"))
		@s_circle.x = 400
		@s_circle.y = 300
		@option_index = 0
	end
	
	def terminate
		Graphics.re.clear
		@renderer.release
	end
	
	def update
		super
		@renderer.clear(HFColorRGBA(0.0, 0.0, 0.0, 0.0))
		@renderer.z_depth = 0.9
		@renderer.draw_texture(@back, HFRect(0, 0, Graphics.width, Graphics.height))
		@renderer.z_depth = 0.1
		@renderer.draw_texture(@option_bar, HFRect(380, 170+@option_index*80, @option_bar.width, @option_bar.height),
						HFRect(0, 0, @option_bar.width, @option_bar.height), 0.5)
		
		@s_circle.angle += 0.1 / MathTool::PI
		@renderer.draw_sprite(@s_circle)
		@renderer.render
	end
	
	def update_keypress
		super
		if @keyboard.is_triggled(DX::DIK_UP)
			@option_index = (@option_index+1)%2
		elsif @keyboard.is_triggled(DX::DIK_DOWN)
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