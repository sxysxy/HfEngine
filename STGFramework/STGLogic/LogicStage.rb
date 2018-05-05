require_relative "../SceneManager.rb"
class LogicStage

	def initialize(s)
		@scene = s
	end

	def update
		check_exit
	end
	
	def check_exit
		SceneManager.return if @scene.keyboard.is_triggled(DX::DIK_ESCAPE)
	end
end