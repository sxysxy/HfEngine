#encoding :utf-8
require 'FFI'
module HEG 

    class Bitmap

        class Rect 
            attr_accessor :x, :y, :width, :height
            def initialize(*arg)
                if arg.size == 4
                    @x = arg[0]
                    @y = arg[1]
                    @width = arg[2]
                    @height = arg[3]
                elsif arg.size == 1 
                    @x = arg[0].x
                    @y = arg[0].y
                    @width = arg[0].width
                    @height = arg[0].height
                elsif arg.size == 0
                    @x = @y = @width = @height = 0
                end
            end
            def pack 
                pack_ints [@x, @y, @width, @height]
            end
        end

        def self.color(r, g, b, a)
            (a << 24) | (b << 16) | (g << 8) | r
        end

        SDL = HEG::FFI::Module.new(File.join(EXECUTIVE_DIRECTORY, "SDL2.dll")) {
            func :SDL_RWFromFile, vptr, [cstr, cstr]
            func :SDL_FreeSurface, int, [vptr]
            func :SDL_RWFromFile, vptr, [cstr, cstr]
            func :SDL_SaveBMP_RW, int, [vptr, vptr, int]
            func :SDL_AllocFormat, vptr, [int]
            func :SDL_ConvertSurface, vptr, [vptr, vptr, int]
            func :SDL_FreeFormat, int, [vptr]
            func :SDL_CreateRGBSurfaceWithFormat, vptr, [int, int, int, int, int]
            func :SDL_UpperBlit, int, [vptr, vptr, vptr, vptr]
            relocate :SDL_BlitSurface, :SDL_UpperBlit
        }

        SDL_Image = HEG::FFI::Module.new(File.join(EXECUTIVE_DIRECTORY, "SDL2_image.dll")) {
            func :IMG_Init, int, [int]
            func :IMG_Load_RW, vptr, [vptr, int]    
        }

        SDL_Image.IMG_Init(0x01 | 0x02 | 0x04 | 0x08) #jpg, png, tiff, webp
        SDL_PIXELFORMAT_RGBA32 = 376840196
        SDL_PIXELFORMAT_RGBA32_STRUCT = SDL.SDL_AllocFormat(SDL_PIXELFORMAT_RGBA32)
 

        attr_reader :native_ptr, :width, :height, :data_addr
        def initialize(*arg)
            @native_ptr = 0
            if arg.size == 2
                @native_ptr = SDL.SDL_CreateRGBSurfaceWithFormat(0, arg[0], arg[1], 32, SDL_PIXELFORMAT_RGBA32)
            elsif arg.size == 1
                if arg[0].is_a?(String)
                    @native_ptr = SDL_Image.IMG_Load_RW(SDL.SDL_RWFromFile(arg[0], "rb"), 1)
                else 
                    @native_ptr = arg[0]
                end
            else 
                raise ArgumentError, "Bitmap.new: requires 1 or 2 argument(s) (Bitmap.new(filename) or Bitmap.new(native_sdl_surface) or Bitmap.new(width, height))"
            end
            if @native_ptr == 0
                raise RuntimeError, "Bitmap.new: Failed to create Bitmap, no enough memory or no such image file"
                return 
            end
            get_info
        end

        def get_info
            @width = FFI.read_int32(@native_ptr+16)
            @height = FFI.read_int32(@native_ptr+20)
            
            if FFI.read_int32(FFI.read_int64(@native_ptr+8)) != SDL_PIXELFORMAT_RGBA32
                op = @native_ptr
                @native_ptr = SDL.SDL_ConvertSurface(op, SDL_PIXELFORMAT_RGBA32_STRUCT, 0)
                SDL.SDL_FreeSurface(op)
            end
            @data_addr = FFI.read_int64(@native_ptr+32)
        end
        private :get_info

        def to_canvas
            HEG::Canvas.new(@width, @height, @data_addr) if @native_ptr
        end

        def release
            if @native_ptr != 0
                SDL.SDL_FreeSurface(@native_ptr)
                @native_ptr = 0
            end
        end

        def blt(dest_x, dest_y, src_bmp)
            dest_rect = Rect.new(dest_x, dest_y, src_bmp.width, src_bmp.height).pack
            SDL.SDL_BlitSurface(src_bmp.native_ptr, 0, @native_ptr, str_ptr(dest_rect))
            self
        end
        def blt_stretch(dest_rect, src_bmp, src_rect)
            dest_rect = dest_rect.pack 
            src_rect = src_rect.pack 
            SDL.SDL_BlitSurface(src_bmp.native_ptr, str_ptr(src_rect), @native_ptr, str_ptr(dest_rect))
            self
        end
    end
end