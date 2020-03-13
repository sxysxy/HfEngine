#encoding: utf-8
require 'FFI'
require 'Bitmap'
module HEG
    module TTF
        SDL = HEG::FFI::Module.new(File.join(EXECUTIVE_DIRECTORY, "SDL2.dll")) {
            func :SDL_RWFromFile, vptr, [cstr, cstr]
            func :SDL_FreeSurface, int, [vptr]
            func :SDL_RWFromFile, vptr, [cstr, cstr]
            func :SDL_SaveBMP_RW, int, [vptr, vptr, int]
            func :SDL_AllocFormat, vptr, [int]
            func :SDL_ConvertSurface, vptr, [vptr, vptr, int]
            func :SDL_FreeFormat, int, [vptr]
        }

        SDL_TTF = HEG::FFI::Module.new(File.join(EXECUTIVE_DIRECTORY, "SDL2_ttf.dll")) {
            func :TTF_Init, int, []
            func :TTF_OpenFont, vptr, [cstr, int]  #TTF_OpenFont(font_filename, font_size) -> native_ptr
            func :TTF_CloseFont, int, [vptr]
            func :TTF_RenderUTF8_Blended, vptr, [vptr, cstr, int]
            func :TTF_RenderUTF8_Shaded, vptr, [vptr, cstr, int, int]
            func :TTF_SetFontStyle, int, [vptr, int]
            func :TTF_SizeUTF8, int, [vptr, cstr, vptr, vptr]
        }

        SDL_TTF.TTF_Init()
        SDL_PIXELFORMAT_RGBA32 = SDL.SDL_AllocFormat(373694468)
        SYSTEM_FONTS_PATH = File.join(WINDOWS_DIRECTORY, "Fonts")

        STYLE_BOLD      = 0x01
        STYLE_ITALIC    = 0x02
        STYLE_UNDERLINE = 0x04
        STYLE_STRIKE    = 0x08

        class Font 
            attr_reader :native_ptr, :file, :style, :size
            def initialize(filename = "msyh.ttc", font_size = 24)
                if !File.exist? filename
                    filename = File.join(SYSTEM_FONTS_PATH, filename)
                    if !File.exist? filename
                        raise RuntimeError, "Could not find font file #{filename} in executive directory or #{SYSTEM_FONTS_PATH}"
                    end
                end
                @file = filename 
                @size = font_size
                reopen
                @style = 0

                @tmp_buf = "\0" * 8
                @buf_ptr = str_ptr(@tmp_buf)
            end

            def reopen
                @native_ptr = SDL_TTF.TTF_OpenFont(@file, @size)
                if @native_ptr == 0
                    raise RuntimeError, "Failed to load font file #{filename}"
                end
            end
            private :reopen

            def release
                if @native_ptr != 0
                    SDL_TTF.TTF_CloseFont @native_ptr
                    @native_ptr = 0
                end
            end

            def draw_text(text, color) 
                sur = SDL_TTF.TTF_RenderUTF8_Blended(@native_ptr, text, color)
                return HEG::Bitmap.new(sur)
            end

            def bold(b = true)
                if b 
                    @style |= STYLE_BOLD
                else 
                    @style &= (~STYLE_BOLD)
                end
                reset_style
                self
            end

            def italic(l = true)
                if l 
                    @style |= STYLE_ITALIC
                else 
                    @style &= (~STYLE_ITALIC)
                end
                reset_style
                self
            end

            def underline(u = true)
                if u
                    @style |= STYLE_UNDERLINE
                else 
                    @style &= (~STYLE_UNDERLINE)
                end
                reset_style
                self
            end

            def strike(s = true)
                if u 
                    @style |= STYLE_STRIKE
                else 
                    @style &= (~STYLE_STRIKE)
                end
                reset_style
                self
            end

            def reset_style
                SDL_TTF.TTF_SetFontStyle(@native_ptr, @style)
            end
            private :reset_style
        
            def size=(s)
                @size = s
                release
                reopen
                reset_style
            end

            def calc_text_size(text)
                SDL_TTF.TTF_SizeUTF8(@native_ptr, text, @buf_ptr, @buf_ptr+4)
                return @tmp_buf.unpack("ii")
            end
        end

        ALIGN_LEFT      = 0x01
        ALIGN_RIGHT     = 0x02
        ALIGN_CENTER    = 0x04
        ALIGN_TOP       = 0x10
        ALIGN_BOTTOM    = 0x20
        ALIGN_VCENTER   = 0x40
        class Blackboard
            attr_reader :line_space
            attr_reader :width, :height
            attr_reader :bitmap
            
            def initialize(w, h, space_between_lines = 4)
                @width = w
                @height = h 
                align(ALIGN_LEFT | ALIGN_VCENTER)
                @line_space = space_between_lines

                @bitmap = Bitmap.new(w, h)
                @posx = 0
                @posy = 0
                @lastx = 0
                @lasty = 0
                @line_heights = []
            end

            def release 
                @bitmap.release
            end

            def line_height(font)
                return @line_space + font.size
            end

            def write(text, specified_font, color)
                @posx = @lastx
                @posy = @lasty
                f = specified_font  
                t = text.to_s.clone
                while !t.empty?
                    c = t.slice!(0, 1)
                    if c == "\n"
                        @posx = 0
                        @posy += line_height(f)
                    elsif c == "\r"
                        next
                    else 
                        cw = f.calc_text_size(c)[0]
                        if @posx + cw >= @width
                            @posx = 0
                            @posy += line_height(f)
                        end
                        tmp = f.draw_text(c, color)
                        draw_x = @posx 
                        draw_y = @posy 
=begin
                        if (@alignment & ALIGN_RIGHT) != 0
                            draw_x = @width - @posx   
                        elsif (@alignment & ALIGN_CENTER) != 0
                            draw_x = width - 
                        end 
=end
                        if (@alignment & ALIGN_VCENTER) != 0
                            draw_y = @posy + @line_space.div(2)
                        elsif (@alignment & ALIGN_BOTTOM) != 0 
                            draw_y = @posy + @line_space  
                        end
                        @bitmap.blt(draw_x, draw_y, tmp)
                        tmp.release
                        @posx += cw
                    end
                end
                @lastx = @posx 
                @lasty = @posy
            end

            def to_canvas
                @bitmap.to_canvas
            end

            def align(a) 
                @alignment = a
            end
            private :process_char

        end
    end 
end