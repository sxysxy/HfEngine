require 'libcore'
require 'Audio'
filename = filebox("choose a music file")

HFWindow.new("playing", 300, 300) {
    show
    set_handler(:on_closed) {exit_mainloop}
    begin 
        Audio.play_bgm(Audio::Sound.new(filename))
    rescue Exception => e
        msgbox e.message
        exit
    end
    messageloop {
        sleep(0.01)
    }
}