begin
    require_relative './Audio/Audio.so'
rescue LoadError => e
    raise LoadError, e.message + "\nNotice: You should put Audio.so into /libruby/Audio \n put SDL2.dll,SDL2_mxier2.dll and other dependencies in Loader.exe's folder\n"
end

Audio.init

