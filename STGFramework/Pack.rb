#encoding: utf-8

require 'fileutils'
require 'scanf'

name = ""
path = ""
if !ARGV[0]
	puts "Input the project name(Only ascii character supported)"
	name = gets.chop
end
if !ARGV[1]
	puts "Input the output directory"
	path = gets.chop.gsub("\\", "/")
end
if !Dir.exist?(path)
	puts "output directory (#{path}) does not exist!(stop)"
	gets
	exit
end
path = File.join(path, name)
if !Dir.exist?(path)
	Dir.mkdir(path)
end
CUR_DIR = File.dirname(__FILE__)
Dir.foreach(CUR_DIR) do |f|
	next if [".", "..", ".gitignore"].include?(f)
	#puts f
	FileUtils.cp_r f, path
end
LOADER_DIR = File.join(CUR_DIR.split("/")[0...-1].join("/"), "Loader")
FileUtils.cp File.join(LOADER_DIR, "Loader.exe"), path
FileUtils.cp File.join(LOADER_DIR, "libcore.rb"), path
FileUtils.cp_r File.join(LOADER_DIR, "libruby"), path

puts "OK!, Input any key to exit"
gets 