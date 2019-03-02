require_relative '../STGLogic/LogicStage.rb'

class SceneStage < Scene
	def start
		@rd_back = G2D::Renderer.new     #using to draw background and the panel
		@rd_content = G2D::Renderer.new  #using to draw content(in battle area)
		@rd_weather = G2D::Renderer.new  #using to draw weather 
		@renderers = [@rd_back, @rd_content, @rd_weather]
	end
	
	def update
		draw_background
		draw_content
		draw_weather
		@renderers.each &:render
	end
	
	def draw_background
		@rd_back.clear(HFColorRGBA(0.0, 0.0, 0.0, 0.0))
	end
	
	def draw_content
	end
	
	def draw_weather
	end
	
	def terminate
		@renderers.each &:release
	end
end