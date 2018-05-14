require_relative "../SceneManager.rb"
class LogicStage

	attr_reader :player_x, :player_y #center pos
	PLAYER_W = 32
	PLAYER_H = 48
	attr_reader :player_dir

	def initialize(s)
		@scene = s
		
		@player_x = 200 - PLAYER_W/2
		@player_y = 420 - PLAYER_H
		@player_dir = 8 #as numpad 8↑ 2↓ 4←，etc
	end

	def update
		check_exit
	end
	
	def check_exit
		SceneManager.return if @scene.keyboard.is_triggled(DX::DIK_ESCAPE)
	end
end