#encoding :utf-8
require_relative "./HighLevelRenderer/Graphics2D.rb"
require "./Controller.rb"

class Scene
	attr_reader :keyboard
	
	def initialize

	end
	
	def main
		start
		while self == SceneManager.scene
			process_message
			Controller.keyboard.update
			update
			Controller.wait_next_frame
		end
		terminate
	end
	
	def start
	
	end
	
	def terminate
	
	end
	
	def update
	
	end
end

module SceneManager
	
	def self.run(scene_klass)
		$window.set_handler(:on_closed) {self.exit}
		@stack = []
		@scene = scene_klass.new
		@scene.main while @scene
		self.clear
	end
	
	def self.scene
		return @scene
	end
	
	def self.exit
		@scene = nil
	end
	
	def self.call(scene_klass)
		@stack.push @scene
		@scene = scene_klass.new
	end
	def self.return 
		@scene = @stack.pop
	end
	
	def self.clear
	
	end
end