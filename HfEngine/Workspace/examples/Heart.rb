require 'HFSF'
HEG::GDevice.create
HEG::Window.new("Heart", 500, 500).instance_exec {
    show 
    context = HEG::RenderContext.new 
    handle(:closed) {break_loop}
    sf = HFSF.loadsf_code {
        Program("Draw") {
            Code(%{
                float4 VS(float2 pos : POS) : SV_POSITION {
                    return float4(pos.x, pos.y, 0.5, 1.0);
                }
                float4 PS(float4 pos : SV_POSITION) : SV_TARGET {
                    float x = (pos.x - 250)/250.0;
                    float y = (pos.y - 200)/250.0;
                    float t = 0;
                    if(abs(x) >= 1e-7) {
                        t = atan(y/x);
                    }
                    else t = acos(-1) / 2;
                    float a = 0.5;
                    
                    if(x <= 0 && abs(x*x + y*y-a*(1-sin(t))) < 1e-2) {
                        return float4(1.0, 0.0, 0.0, 1.0);
                    }else if(x >= 0 && abs(x*x+y*y-a*(1+sin(t))) < 1e-2) {
                        return float4(1.0, 0.0, 0.0, 1.0);
                    } else {
                        return float4(1.0, 1.0, 1.0, 1.0);
                    }
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
    vb = HEG::VertexBuffer.new(4, 4 * 2, pack_floats([-1.0, 1.0, 1.0, 1.0, -1.0, -1.0, 1.0, -1.0]))
    ib = HEG::IndexBuffer.new(6, pack_ints([0, 1, 2, 1, 3, 2]))
    context.vbuffer(vb).ibuffer(ib).
                viewport(0, 0, width(), height()).
                target(canvas())
    timer = HEG::FPSTimer.new(HEG::GDevice.instance.monitor_info[:refresh_rate])
    mainloop {
        context.clear
        context.draw_index(HEG::TOPOLOGY_TRIANGLES, 0, 6)
        context.render
        timer.wait
        swap_buffers()
    }
}