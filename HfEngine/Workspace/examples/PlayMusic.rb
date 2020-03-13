require 'EasyAudio'
require 'Extends'
include HEG
CURRENT_PATH = File.dirname(__FILE__)
filename = Filebox.show({:title => "Choose a music file", 
    :filter => ["Any file(*.*)", "*.*", "FLAC(*.flac)", "*.flac", "MP3(*.mp3)", "*.mp3", 
        "OGG(*.ogg)", "*.ogg"]})[0]
if !filename 
    msgbox "bye", "You did not choose any file"
    exit_process 0
end
begin 
    music = Audio::Music.new(filename)
    Audio.play_bgm(music)
    show_console
    puts "Now playing #{filename}", "When you do not want to listen to it, just press any key\n"
    system "pause"
    Audio.stop_bgm
    music.release
    exit_process 0
rescue Exception => e   
    msgbox "Oh", "#{filename} seems not to be a music file,\n or its format is not supported: #{e.message}"
end