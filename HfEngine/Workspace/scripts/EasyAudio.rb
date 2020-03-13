#encoding: utf-8
require 'FFI'
module HEG
    module Audio

        SDL = HEG::FFI::Module.new(File.join(EXECUTIVE_DIRECTORY, "SDL2.dll")) {
            func :SDL_Init, int, [int]
            func :SDL_RWFromFile, vptr, [cstr, cstr]
        }

        SDLMixer = HEG::FFI::Module.new(File.join(EXECUTIVE_DIRECTORY, "SDL2_mixer.dll")) {
            func :Mix_Init, int, [int]
            func :Mix_OpenAudio, int, [int, int, int, int]
            func :Mix_AllocateChannels, int, [int]
            func :Mix_VolumeMusic, int, [int]
            func :Mix_SetMusicPosition, int, [int]
            func :Mix_PlayMusic, int, [vptr, int]
            func :Mix_PlayingMusic, int, []
            func :Mix_PauseMusic, int, []
            func :Mix_HaltMusic, int, []
            func :Mix_ResumeMusic, int, []
            func :Mix_FadeOutMusic, int, [int]
            func :Mix_PlayChannelTimed, int, [int, vptr, int, int]
            func :Mix_HaltChannel, int, [int]
            func :Mix_VolumeChunk, int, [int, int]
            func :Mix_HaltChannel, int, [int]
            func :Mix_LoadMUS, vptr, [cstr]
            func :Mix_LoadWAV_RW, vptr, [vptr, int]
            func :Mix_FreeMusic, int, [vptr]
            func :Mix_FreeChunk, int, [vptr]
        }

        if SDL.SDL_Init(0x10) == -1 
            raise RuntimeError, "File to init SDL2"
        end
        SDLMixer.Mix_Init(1 | 32 | 2 | 8 | 16 | 64)
        SDLMixer.Mix_OpenAudio(44100, 0x8010, 2, 4096)
        SDLMixer.Mix_AllocateChannels(16)

        class Music 
            attr_reader :native_ptr           
            def initialize(filename)
                @native_ptr = SDLMixer.Mix_LoadMUS(filename)    
                (raise LoadError, "Could not load music #{filename}") if 0 == @native_ptr
            end
            def release
                SDLMixer.Mix_FreeMusic(@native_ptr)
                @native_ptr = 0
            end
        end 

        class Effect
            attr_reader :native_ptr
            def initialize(filename)
                @native_ptr = SDLMixer.Mix_LoadWAV_RW(SDL.SDL_RWFromFile(filename, "rw"), 1)
                (raise LoadError, "Could not load sound effect #{}") if 0 == @native_ptr
                @volumn = Audio.default_se_volumn
            end
            def release
                SDLMixer.Mix_FreeChunk(@native_ptr) if 0 != @native_ptr
                @native_ptr = 0
            end
        end

        @@default_bgm_volumn = 20
        @@default_se_volumn = 20

        def self.play_bgm(music, loop = false, volumn = nil, pos = nil)
            SDLMixer.Mix_VolumeMusic(volumn ? volumn : @@default_bgm_volumn)
            SDLMixer.Mix_SetMusicPosition(pos) if pos
            SDLMixer.Mix_PlayMusic(music.native_ptr, loop ? -1 : 1)
        end 

        def self.bgm_volumn
            return @@default_bgm_volumn
        end
        def self.se_volumn
            return @@se_volumn
        end
        def self.bgm_volumn=(v)
            @@default_bgm_volumn = v.to_i
        end
        def self.se_volumn=(v)
            @@default_se_volumn = v.to_i
        end

        def self.playing_bgm?
            SDLMixer.Mix_PlayingMusic() == 0 ? false : true
        end

        def self.pause_bgm
            SDLMixer.Mix_PauseMusic()
        end

        def self.resume_bgm 
            SDLMixer.Mix_HaltMusic()
        end

        def self.fadeout_bgm(fade_time = 1000)
            SDLMixer.Mix_FadeOutMusic(fade_time)
        end

        def self.stop_bgm
            SDLMixer.Mix_HaltMusic()
        end

        def self.play_se(effect)
            @se_channel = SDLMixer.Mix_PlayChannel(-1, effect.native_ptr, 0, -1)
        end

        def self.stop_se
            SDLMixer.Mix_HaltChannel(@se_channel) if @se_channel
        end
    end 
end