Dir.chdir File.dirname(__FILE__)

require "./ConfigLoader.rb"
require "./TextureCache.rb"

config = ConfigLoader.load("./config.rb")

