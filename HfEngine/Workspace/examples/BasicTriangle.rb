#encoding: utf-8
require 'HFSF'
HEG::GDevice.create
HEG::Window.new("Basic Demo", 800, 600).instance_exec {
    show()
    handle(:closed) { break_loop }
    context = HEG::RenderContext.new
    #Load Shader Framework
    sf = HFSF.loadsf_code {
        Program("Draw") {
            Code(%{
                float4 VS(float2 pos : POS) : SV_POSITION {
                    return float4(pos.x, pos.y, 0.5, 1.0);
                }
                float4 PS(float4 pos : SV_POSITION) : SV_TARGET {
                    return float4(0.8, 0.5, 0.3, 1.0);
                }
            })
            InputLayout { format "POS", HEG::FORMAT_FLOAT2 }
            Section("set") {
                vshader("VS")
                pshader("PS")
            }
        }
    }
    sf.section[:set].apply(context)
    sf.layout.apply(context)
    vb = HEG::VertexBuffer.new(3, 4 * 2, pack_floats([0.0, 1.0, -1.0, -1.0, 1.0, -1.0]))
    context.vbuffer(vb).
                viewport(0, 0, width(), height()).
                target(canvas())
    timer = HEG::FPSTimer.new(HEG::GDevice.instance.monitor_info[:refresh_rate] * 2)
    mainloop {
        context.clear
        context.draw(HEG::TOPOLOGY_TRIANGLES, 0, 3)
        context.render
        timer.wait
        swap_buffers()
    }
    vb.release
    sf.release
    context.release
    HEG::GDevice.release
}