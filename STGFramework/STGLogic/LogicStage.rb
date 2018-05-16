require_relative "../SceneManager.rb"
class LogicStage

	attr_reader :player_x, :player_y #center pos
	PLAYER_W = 32
	PLAYER_H = 48
	attr_reader :player_dir
	attr_reader :shooting, :slow

	def initialize
		@player_x = 200 - PLAYER_W/2
		@player_y = 420 - PLAYER_H
		@player_dir = :up #
		@shooting = false
		@slow = false
		@keyboard = Controller.keyboard
		
	end

	def update
		check_exit
		update_keypress
	end
	
	def update_keypress
		@speed = 6
		@player_dir = :up
		@shooting = @slow = false
		@shooting = true if @keyboard.is_pressed_now(DX::DIK_Z)
		
	    if @keyboard.is_pressed_now(DX::DIK_LSHIFT)
			@speed = 4
			@slow = true
		end
		if ([@keyboard.is_pressed_now(DX::DIK_LEFT),
			@keyboard.is_pressed_now(DX::DIK_RIGHT),
			@keyboard.is_pressed_now(DX::DIK_UP),
			@keyboard.is_pressed_now(DX::DIK_DOWN)].map {|x| x ? 1: 0}).sum >= 2
			@speed *= Math.sqrt(2) / 2;
		end
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