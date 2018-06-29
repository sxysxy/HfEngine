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
		@sp_panel = Sprite.new TextureCache.load("./stage/back.png")
		
		Graphics.re.insert(@rd_back, 200)
	end
	def init_content
		@reimus = TextureCache.load("/th14/player/pl00/pl00.png")
		@shoots = TextureCache.load("/th14/player/pl00/shoots.png")
		
		@sp_reimu_forward = Sprite.new(DX::Texture2D.new($device, 32, 48))
		@sp_reimu_left = Sprite.new(DX::Texture2D.new($device, 32, 48))
		@sp_reimu_right = Sprite.new(DX::Texture2D.new($device, 32, 48))
		@sp_reimu_back = Sprite.new(DX::Texture2D.new($device, 32, 49))
		@sp_point = Sprite.new(TextureCache.load("/th14/player/point.png"))
		@sp_shoots = Sprite.new(DX::Texture2D.new($device, 63, 12*40))
		
		Graphics.re.lock
		@rd_content.immdiate_copy2d(@sp_reimu_forward.texture, @reimus, HFRect(0, 0, 32, 48), 
																		HFRect(32, 0, 32, 48))
		@rd_content.immdiate_copy2d(@sp_reimu_left.texture, @reimus, HFRect(0, 0, 32, 48), 
																		HFRect(96, 48, 32, 48))
		@rd_content.immdiate_copy2d(@sp_reimu_right.texture, @reimus, HFRect(0, 0, 32, 48), 
																		HFRect(96, 96, 32, 48))
		@rd_content.immdiate_copy2d(@sp_reimu_back.texture, @reimus, HFRect(0, 0, 32, 48), 
																		HFRect(32, 48, 32, 48))
		40.times do |i|
			@rd_content.immdiate_copy2d(@sp_shoots.texture, @shoots, HFRect(0, i*12, 63, 12),
																	HFRect(0, 0, 63, 12))
		end
		@sp_shoots.zoom_x = 0.8
		Graphics.re.unlock
		Graphics.re.insert(@rd_content, 100)
		[@sp_reimu_forward, @sp_reimu_back, @sp_reimu_left, @sp_reimu_right, @sp_point, 
			@sp_shoots].each {|s|
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
		[@sp_reimu_forward, @sp_reimu_back, @sp_reimu_left, @sp_reimu_right, @sp_shoots].each &:release_texture
	end
	
	derive(:draw_background) 
	def draw_background
		super
		@rd_back.draw_sprite(@sp_panel)
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
			@sp_point.x = sp.x + sp.width / 2 - 4
			@sp_point.y = sp.y + sp.height / 2 - 4
			@rd_content.draw_sprite(@sp_point)
		end
		if @logic.shooting
			@sp_shoots.x = sp.x - 8
			@sp_shoots.y = sp.y - 12 * 40 - 24
			dy = rand(-12..12)
			@sp_shoots.oy -= dy
			@sp_shoots.y += dy
			@sp_shoots.opacity = [0.8, 0.6, 1.0, 1.0, 1.0, 0.7].sample
			@rd_content.draw_sprite(@sp_shoots)
			@sp_shoots.oy += dy
			@sp_shoots.y -= dy
		end
	end
end