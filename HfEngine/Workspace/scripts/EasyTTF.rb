#encoding: utf-8
require 'FFI'
module HEG
    module TTF
        SDL = HEG::FFI::Module.new("SDL2.dll") {
            func :SDL_RWFromFile, vptr, [cstr, cstr]
            func :SDL_FreeSurface, int, [vptr]
            func :SDL_RWFromFile, vptr, [cstr, cstr]
            func :SDL_SaveBMP_RW, int, [vptr, vptr, int]
        }

        SDL_TTF = HEG::FFI::Module.new("SDL2_ttf.dll") {
            func :TTF_Init, int, []
            func :TTF_OpenFont, vptr, [cstr, int]  #TTF_OpenFont(font_filename, font_size) -> native_ptr
            func :TTF_CloseFont, int, [vptr]
            func :TTF_RenderUTF8_Blended, vptr, [vptr, cstr, int]
        }

        SDL_TTF.TTF_Init()
        class Font 
            attr_reader :native_ptr, :file
            def initialize(filename, size = 24)
                if !File.exist? filename
                    filename = File.join("C:/Windows/Fonts", filename)
                    if !File.exist? filename
                        raise RuntimeError, "Could not find font file #{filename} in executive directory or C:/windows/fonts"
                    end
                end
                @file = filename
                @native_ptr = SDL_TTF.TTF_OpenFont(filename, size)
                if @native_ptr == 0
                    raise RuntimeError, "Failed to load font file #{filename}"
                end
            end

            def draw_text(text, color) 
                sur = SDL_TTF.TTF_RenderUTF8_Blended(@native_ptr, text, color)
                w = FFI.read_int32(sur+16)
                h = FFI.read_int32(sur+20)
                #msgbox("#{w} #{h}")
                data = FFI.read_int64(sur+32)
                #msgbox "#{data}"
                tex = HEG::Canvas.new(w, h, data)
                SDL.SDL_SaveBMP_RW(sur, SDL.SDL_RWFromFile("fuck.bmp", "wb"), 1)
                SDL.SDL_FreeSurface(sur)
                return tex
            end

        end
    end 
end