#encoding :utf-8
=begin
Basic Modules of the game.
Including these Modules:
    Controller: 
        Controls logical FPS. 
    SceneManager:
        Manage game scenes.
    SoundManager:
        Manage game sounds.
    TextureCache:
        Cache of textures.
=end


=begin rdoc
 Module Controller. Controls logical FPS
=end
module Controller
	
    # Initialize this module
	def self.init
		@fps = $config[:logic].fps or 60
		@timer = FPSTimer.new(@fps)
		
		@keyboard = DX::Input::Keyboard.new($window)
	end
    
    # Get game logical fps
	def self.fps
		return @fps
    end
    
    # Set game logical fps
	def self.fps=(f)
		@fps = f
		restart
    end
    
    # Wait until next logical frame 
	def self.wait_next_frame
		@timer.await
    end

    # Get keyboard object
    def self.keyboard
		return @keyboard
    end
    
    # Stop controller and release resource
	def self.shutdown
		@keyboard.release
	end

    private
	def self.restart
		@timer.restart(@fps)
	end
end


=begin rdoc
Class Scene, Basic Scene abstraction 
=end
class Scene	

	# Initialize the scene
	def initialize
	end

	# The main proc of the scene
	# It calls start, then loop and update, after the loop finish, it calls terminate 
	def main
		start
		while self == SceneManager.scene
			Graphics.update
			Controller.keyboard.update
			update
			Controller.wait_next_frame
		end
		terminate
	end
	
	# Called before scene's main loop
    def start
        
	end
	
	# Called after scene's main loop
    def terminate
        
	end
	
	# Called by main loop each frame
	def update
	end
end

=begin rdoc
Manage scenes.
=end
module SceneManager

	# Initialize and run the SceneManager from specific scene
	# Parameter 'scene_klass' is a Scene class which will run.
	def self.run(scene_klass)
		$window.set_handler(:on_closed) {self.exit}
		@stack = []
		@scene = scene_klass.new
		@scene.main while @scene
		self.clear
	end
	
	# Get Current scene
	def self.scene
		return @scene
	end
	
	# Stop SceneManager
	def self.exit
		@scene = nil
	end
	
	# Call a scene.
	def self.call(scene_klass)
		@stack.push @scene
		@scene = scene_klass.new
	end

	# Return to previous scene
	def self.return 
		@scene = @stack.pop
	end
	
	# Clear scene call stack
	def self.clear
		@stack.clear
	end
end

=begin rdoc
Manage game sound
=end
require 'Audio'
module SoundManager 
    include Audio
    class << self
        attr_accessor :se_volume, :bgm_volume
		
		# play bgm
        def play_bgm(filename)
            Audio.play_bgm load_bgm(filename), true, @bgm_volume 
        end

		# play sound effect
        def play_se(filename) 
            Audio.play_se load_se(filename), @se_volume
        end

		# load sound effect into memory and get an Audio::Effect object
        def load_se(filename)
            @se = Audio::Effect.new(File.join(MAIN_DIR, "/Resources/se/#{filename}"))
        end

		# load bgm into memory and get an Audio::Sound object 
        def load_bgm(filename)
            @bgm = Audio::Sound.new(File.join(MAIN_DIR, "/Resources/bgm/#{filename}"))
        end
    end
end

=begin rdoc
Cache of texutes.
This module can cache a texture in memory when you load it for the first time.
If you load some textures very frequently, this module can help reduce disk IO operations.
=end
module TextureCache
	# The directory of resources.
	RESOURCE_PATH = File.join(MAIN_DIR, "Resources/")
	
	# Load a picture form cache or disk. 
	# filename's base directory is set to RESOURCE_PATH
	def self.load(filename)
		@textures ||= {}
		
		fn = File.join(RESOURCE_PATH, filename)
		@textures[fn] or (@textures[fn] = DX::Texture2D.new($device, fn))
	end	

	#Clear caches and release all textures loaded.
	def self.clear
		return if !@textures 
		@textures.each {|f, t|
			t.release
		}
		@textures = {}
	end
end
