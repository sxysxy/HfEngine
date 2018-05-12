require_relative "../CommonScene/SceneStage.rb"

class SceneSHWStage < SceneStage
	def start
		super
		init_back
		init_content
	end
	
	def init_back
		@sp_panel = TextureCache.load("./stage/back.png")
		
		Graphics.re.insert(@rd_back, 200)
	end
	def init_content
		@reimus = TextureCache.load("/th14/player/pl00/pl00.png")
		
		@reimu_forward, @reimu_left, @reimu_right, @reimu_back = Array.new(4) {DX::Texture2D.new($device, 32, 48)}
=begin
		Graphics.re.lock
		@rd_content.set_target(DX::RTT.new(@reimu_forward))
		@rd_content.draw_texture(@reimus, HFRect(0, 0, 32, 48), HFRect(32, 0, 32, 48))
		@rd_content.immdiate_render
		@rd_content.use_default_target
		Graphics.re.unlock
=end
		Graphics.re.insert(@rd_content, 100)
	end
	
	def terminate
		super
		Graphics.re.clear
		[@reimu_forward, @reimu_back, @reimu_left, @reimu_right].each &:release
	end
	
	derive(:draw_background) 
	def draw_background
		super
		@rd_back.draw_texture(@sp_panel, HFRect(0, 0, @sp_panel.width, @sp_panel.height))
	end
	
	derive(:draw_content)
	def draw_content
		#@rd_content.set_target(DX::RTT.new(@reimu_forward))
		#@rd_content.draw_texture(@reimus, HFRect(0, 0, 32, 48), HFRect(32, 0, 32, 48))
		#@rd_content.use_default_target
		#@rd_content.draw_texture(@reimu_forward, HFRect(100, 100, 32, 48), HFRect(0, 0, 32, 48))
		#@rd_content.draw_texture(@reimus, HFRect(100, 100, 32, 48), HFRect(0, 0, 32, 48))
		@rd_content.set_target(DX::RTT.new(@reimu_forward))
		@rd_content.set_viewport(HFRect(0, 0, @reimu_forward.width, @reimu_forward.height))
		@rd_content.draw_texture(@reimus, HFRect(0, 0, 32, 48), HFRect(32, 0, 32, 48))
		@rd_content.use_default_target
		@rd_content.set_viewport(HFRect(0, 0, $window.width, $window.height))
		@rd_content.draw_texture(@reimu_forward, HFRect(100, 100, 32, 48))
	end
end