require_relative "../CommonScene/SceneStage.rb"
require_relative '../STGLogic/LogicStage.rb'

class SceneSHWStage < SceneStage
	def start
		super
		init_back
		init_content
		
		@logic = LogicStage.new(self)
	end
	
	def init_back
		@sp_panel = TextureCache.load("./stage/back.png")
		
		Graphics.re.insert(@rd_back, 200)
	end
	def init_content
		@reimus = TextureCache.load("/th14/player/pl00/pl00.png")
		Graphics.re.insert(@rd_content, 100)
		
		@sp_player = Sprite.new(@reimus)
		@sp_player.viewport = HFRect(25, 25, 400, 420)
	end
	
	def update
		@logic.update
		super
	end
	
	def terminate
		super
		Graphics.re.clear 
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
		@sp_player.src_rect = HFRect(32, 0, 32, 48)
		@sp_player.x = @logic.player_x - 16
		@sp_player.y = @logic.player_y - 24
		@sp_player.zoom_x = 32.0 / @reimus.width
		@sp_player.zoom_y = 48.0 / @reimus.height
		@rd_content.draw_sprite(@sp_player)
	end
end