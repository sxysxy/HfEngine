#encoding: utf-8
require './libcore.rb'
HFWindow.new("Demo", 500, 500) { 
    show
    set_handler(:on_closed) {exit_mainloop}
    timer = FPSTimer.new(60) 
	messageloop{
		timer.await
	}
}

