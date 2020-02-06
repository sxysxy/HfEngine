#encoding: utf-8
HEG::GDevice.create

=begin
User32 = HEG::FFI::dlopen("user32.dll")
show_console
puts User32.to_i
=end
show_console
puts HEG::GDevice.instance.adapter_info

HEG::Window.new("aaa", 800, 600).instance_exec {
    fixed false 
    show
    handle(:closed) { exit_process 0 }
    timer = HEG::FPSTimer.new(HEG::GDevice.instance.monitor_info[:refresh_rate])
    mainloop {
        timer.wait
        swap_buffers
    }
}