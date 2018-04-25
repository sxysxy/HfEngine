#encoding :utf-8
Dir.chdir File.dirname(__FILE__)

require 'libcore'
require "./ConfigLoader.rb"
require "./TextureCache.rb"
require "./SceneManager.rb"
require "./Controller.rb"
require_relative "./STGScene/SceneTitle.rb"

$config = ConfigLoader.load("./config.rb")

title = $config[:graphics].title or "A STG Game"
width, height = $config[:graphics].resolution ? $config[:graphics].resolution : [640, 480]
hardware = $config[:graphics].driver_type == :HARDWARE ? DX::HARDWARE_DEVICE : DX::SIMULATED_DEVICE

$window = HFWindow.new(title, width, height)
$window.show
$device = DX::D3DDevice.new(hardware)
Controller.init
Graphics.init
SceneManager.run(SceneTitle)