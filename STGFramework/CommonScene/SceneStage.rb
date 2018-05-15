require_relative '../HighLevelRenderer/Renderer2D.rb'
require_relative '../STGLogic/LogicStage.rb'

class SceneStage < Scene
	def start
		@rd_back = Renderer2D.new  #using to draw background and the panel
		@rd_content = Renderer2D.new  #using to draw content(in battle area)
		@rd_weather = Renderer2D.new  #using to draw weather 
		@rd_back.z_depth = 0.9
		@rd_content.z_depth = 0.5
		@rd_weather.z_depth = 0.1
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