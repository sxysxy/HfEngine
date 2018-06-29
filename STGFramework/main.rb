#encoding :utf-8
Dir.chdir File.dirname(__FILE__)
show_console
class Class
	def derive(m)
		self.superclass.instance_method(m)
	end
end

require 'libcore'
require_relative "./ConfigLoader.rb"
require_relative "./TextureCache.rb"
require_relative "./SceneManager.rb"
require_relative "./Controller.rb"

require_lib "Graphics2D.rb"

if File.exist?("./STGScene/SceneSHWTitle.rb")
	require_relative "./STGScene/SceneSHWTitle.rb"
	TITLE_CLASS = SceneSHWTitle
else
	require_relative "./CommonScene/SceneTitle.rb"
	TITLE_CLASS = SceneTitle
end
require_relative "./HighLevelRenderer/Renderer2D.rb"

$config = ConfigLoader.load("./config.rb")

title = $config[:graphics].title or "A STG Game"
width, height = $config[:graphics].resolution ? $config[:graphics].resolution : [640, 480]

$graphics = G2D::Graphics.new(title, width, height, $config[:graphics].vsync ? (:vsync) : ($config[:graphics].fps ? $config[:graphics].fps : 60))
Graphics = $graphics

$window = $graphics.window
$device = $graphics.device
Controller.init
SceneManager.run(TITLE_CLASS)

#do release
Controller.shutdown
TextureCache.clear
$device.release