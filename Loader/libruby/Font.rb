begin
    require_relative './Font/Font.so'
rescue LoadError => e
    raise LoadError, e.message + "\nNotice: You should put Font.so into /libruby/Font \n put SDL2.dll and other dependencies in Loader.exe's folder\n"
end

Font.init

class Font 
    attr_reader :bold
    attr_reader :italic
    attr_reader :underline
    attr_reader :strike_through
    attr_reader :size
end