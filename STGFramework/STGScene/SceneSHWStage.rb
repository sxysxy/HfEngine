require_relative "../CommonScene/SceneStage.rb"
require_relative '../STGLogic/LogicStage.rb'

class SceneSHWStage < SceneStage
	def start
		super
		init_back
		init_content
		
		@logic = LogicStage.new
	end
	
	def init_back
		@sp_panel = TextureCache.load("./stage/back.png")
		
		Graphics.re.insert(@rd_back, 200)
	end
	def init_content
		@reimus = TextureCache.load("/th14/player/pl00/pl00.png")
		
		@sp_reimu_forward = Sprite.new(DX::Texture2D.new($device, 32, 48))
		@sp_reimu_left = Sprite.new(DX::Texture2D.new($device, 32, 48))
		@sp_reimu_right = Sprite.new(DX::Texture2D.new($device, 32, 48))
		@sp_reimu_back = Sprite.new(DX::Texture2D.new($device, 32, 49))
		@sp_point = Sprite.new(TextureCache.load("/th14/player/point.png"))
		#@sp_shoot = Sprite.new(DX::Texture2D.new($device, 40, 16*27)
		
		Graphics.re.lock
		@rd_content.immdiate_copy2d(@sp_reimu_forward.texture, @reimus, HFRect(0, 0, 32, 48), 
																		HFRect(32, 0, 32, 48))
		@rd_content.immdiate_copy2d(@sp_reimu_left.texture, @reimus, HFRect(0, 0, 32, 48), 
																		HFRect(96, 48, 32, 48))
		@rd_content.immdiate_copy2d(@sp_reimu_right.texture, @reimus, HFRect(0, 0, 32, 48), 
																		HFRect(96, 96, 32, 48))
		@rd_content.immdiate_copy2d(@sp_reimu_back.texture, @reimus, HFRect(0, 0, 32, 48), 
																		HFRect(32, 48, 32, 48))
		
		Graphics.re.unlock
		Graphics.re.insert(@rd_content, 100)
		[@sp_reimu_forward, @sp_reimu_back, @sp_reimu_left, @sp_reimu_right, @sp_point].each {|s|
			s.viewport = HFRect(25, 25, 400, 420)
			s.origin_center
			s.z = 0.5
		}
		@dir2sp = {:up => @sp_reimu_forward, :down => @sp_reimu_back, :left => @sp_reimu_left, :right => @sp_reimu_right}
	end
	
	def update
		@logic.update
		super
	end
	
	def terminate
		super
		Graphics.re.clear 
		[@sp_reimu_forward, @sp_reimu_back, @sp_reimu_left, @sp_reimu_right].each &:release_texture
	end
	
	derive(:draw_background) 
	def draw_background
		super
		@rd_back.draw_texture(@sp_panel, HFRect(0, 0, @sp_panel.width, @sp_panel.height))
	end
	
	derive(:draw_content)
	def draw_content
		draw_player
	end
	
	def draw_player
		sp = @dir2sp[@logic.player_dir]
		sp.x = @logic.player_x
		sp.y = @logic.player_y
		@rd_content.draw_sprite(sp)
		if @logic.slow
			@sp_point.x = sp.x
			@sp_point.y = sp.y
			@rd_content.draw_sprite(@sp_point)
		end
	end
end