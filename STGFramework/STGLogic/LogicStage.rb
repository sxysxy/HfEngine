require_relative "../SceneManager.rb"
class LogicStage

	attr_reader :player_x, :player_y #center pos
	PLAYER_W = 32
	PLAYER_H = 48
	attr_reader :player_dir

	def initialize
		@player_x = 200 - PLAYER_W/2
		@player_y = 420 - PLAYER_H
		@player_dir = :up #
		
		@keyboard = Controller.keyboard
	end

	def update
		check_exit
		update_keypress
	end
	
	def update_keypress
		@speed = 6
		@speed = 4 if @keyboard.is_pressed_now(DX::DIK_LSHIFT)
		if @keyboard.is_pressed_now(DX::DIK_LEFT)
			@player_x -= @speed 
			@player_dir = :left
		end
		if @keyboard.is_pressed_now(DX::DIK_RIGHT)
			@player_x += @speed 
			@player_dir = :right
		end
		if @keyboard.is_pressed_now(DX::DIK_UP)
			@player_y -= @speed 
			@player_dir = :up
		end
		if @keyboard.is_pressed_now(DX::DIK_DOWN)
			@player_y += @speed 
			@player_dir = :down
		end
		@player_x = -6 if @player_x < -6
		@player_x = 392 - PLAYER_W if @player_x > 392 - PLAYER_W
		@player_y = 0 if @player_y < 0
		@player_y = 420 - PLAYER_H if @player_y > 420 - PLAYER_H
	end
	
	def check_exit
		SceneManager.return if Controller.keyboard.is_triggled(DX::DIK_ESCAPE)
	end
end