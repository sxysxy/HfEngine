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
		@rd_content.set_viewport(HFRect(25, 25, 400, 425))
		Graphics.re.insert(@rd_content, 100)
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
		@rd_content.draw_texture(@reimus, HFRect(100, 100, 32, 48), HFRect(0, 0, 32, 48))
	end
end