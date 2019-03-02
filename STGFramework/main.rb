#encoding :utf-8
MAIN_DIR = File.dirname(__FILE__)
Dir.chdir MAIN_DIR
class Class
	def derive(m)
		self.superclass.instance_method(m)
	end
	alias override derive
end

require 'libcore'
require "ConfigLoader"
require "Graphics2D.rb"
require_relative "./Scripts/GameBasic.rb"
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

$graphics = Graphics.init(title, width, height, $config[:graphics].vsync ? (:vsync) : ($config[:graphics].fps ? $config[:graphics].fps : 60))
if $config[:graphics].fullscreen 
	$graphics.fullscreen
end

SoundManager.se_volume = $config[:audio].se_volume or 20
SoundManager.bgm_volume = $config[:audio].bgm_volume or 20

begin
	$window = $graphics.window
	$device = $graphics.device
	Controller.init
	SceneManager.run(TITLE_CLASS)
rescue Exception => e
	msgbox $@.to_s + e.message
ensure #do release
	Controller.shutdown
	TextureCache.clear
	$device.release
end
