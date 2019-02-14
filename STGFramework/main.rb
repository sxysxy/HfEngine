#encoding :utf-8
Dir.chdir File.dirname(__FILE__)
class Class
	def derive(m)
		self.superclass.instance_method(m)
	end
	alias override derive
end

require 'libcore'
require_relative "./ConfigLoader.rb"
require_relative "./TextureCache.rb"
require_relative "./SceneManager.rb"
require_relative "./Controller.rb"

require "Graphics2D.rb"
include G2D

if File.exist?("./STGScene/SceneSHWTitle.rb")
	require_relative "./STGScene/SceneSHWTitle.rb"
	TITLE_CLASS = SceneSHWTitle
else
	require_relative "./CommonScene/SceneTitle.rb"
	TITLE_CLASS = SceneTitle
end
$config = ConfigLoader.load("./config.rb")
show_console if $config[:graphics].console

title = $config[:graphics].title or "A STG Game"
width, height = $config[:graphics].resolution ? $config[:graphics].resolution : [640, 480]

$graphics = G2D::Graphics.init(title, width, height, $config[:graphics].vsync ? (:vsync) : ($config[:graphics].fps ? $config[:graphics].fps : 60))
Graphics = $graphics
if $config[:graphics].fullscreen 
	Graphics.fullscreen
end

begin
	$window = $graphics.window
	$device = $graphics.device
	Controller.init

	SceneManager.run(TITLE_CLASS)
rescue Exception => e
	msgbox e.message
ensure #do release
	Controller.shutdown
	TextureCache.clear
	$device.release
end