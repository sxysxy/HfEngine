#encoding :utf-8
Dir.chdir File.dirname(__FILE__)
show_console
class Class
	def derive(m)
		self.superclass.instance_method(m)
	end
end

require 'libcore'
require "./ConfigLoader.rb"
require "./TextureCache.rb"
require "./SceneManager.rb"
require "./Controller.rb"

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

$window = HFWindow.new(title, width, height)
$window.show
$device = DX::D3DDevice.new
Controller.init
Graphics.init
SceneManager.run(TITLE_CLASS)

#do release
Graphics.shutdown
TextureCache.clear
$device.release