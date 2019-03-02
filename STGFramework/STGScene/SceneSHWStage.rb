require_relative "../CommonScene/SceneStage.rb"

class SceneSHWStage < SceneStage
	include G2D
	def start
		super
		init_back
		init_content
	end
	
	def init_back
	end
	def init_content
	end
	
	def update
		super
	end
	
	def terminate
		super
		Graphics.re.clear
	end
	
	derive(:draw_background) 
	def draw_background
		super
	end
	
	derive(:draw_content)
	def draw_content
		super
	end
	
	def draw_player
	end
end