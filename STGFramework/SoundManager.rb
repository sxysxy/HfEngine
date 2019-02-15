#encoding:utf-8
#Manage game sound
require 'Audio'

module SoundManager 
    include Audio
    class << self
        attr_accessor :se_volume, :bgm_volume

        def play_bgm(filename)
            Audio.play_bgm load_bgm(filename), true, @bgm_volume 
        end

        def play_se(filename) 
            Audio.play_se load_se(filename), @se_volume
        end

        def load_se(filename)
            @se = Audio::Effect.new(File.join(MAIN_DIR, "/Resources/se/#{filename}"))
        end

        def load_bgm(filename)
            @bgm = Audio::Sound.new(File.join(MAIN_DIR, "/Resources/bgm/#{filename}"))
        end
    end
end