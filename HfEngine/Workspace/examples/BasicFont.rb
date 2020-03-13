require 'EasyTTF'
require 'HFSF'
include HEG
GDevice.create
Window.new("Simple Font Rendering Demo", 800, 30).instance_exec {
    show 
    handle(:closed) { break_loop }
    
    font = TTF::Font.new("msyh.ttc").bold.italic
    bmp = font.draw_text("abcdefghigklmnnopqrstuvwxyz1234567890一二三四五六七八九十", Bitmap.color(255, 255, 0, 255)); #纯黄 颜色
    font.release
    tex = bmp.to_canvas
    bmp.release
    context = RenderContext.new.viewport(0, 0, width(), height()).target(canvas())
    sf = HFSF.loadsf_code {
        Program("Draw") {
            Code(%{
                struct vsout {
                    float4 position : SV_POSITION;
                    float2 texcoord : TEX;
                };
            
                vsout VS(float2 pos : POS, float2 tex : TEX) {
                    vsout opt;
                    opt.position = float4(pos, 0.0, 1.0);
                    opt.texcoord = tex;
                    return opt;
                }

                Texture2D tex : register(t0);
                SamplerState color_sampler : register(s0);

                float4 PS(vsout vo) : SV_TARGET {
                    float4 s = tex.Sample(color_sampler, vo.texcoord);
                    return s;
                    
                }
            })
            InputLayout {
                format "POS", HEG::FORMAT_FLOAT2
                format "TEX", HEG::FORMAT_FLOAT2
            }
            Resource {
                Sampler("sampler") {}
            }
            Section("draw") {
                vshader("VS")
                pshader("PS")
                ps_sampler(0, "sampler")
            }
        }
    }
    sf.section[:draw].apply(context)
    sf.layout.apply(context)
    vb = VertexBuffer.new(4, 4 * 4, pack_floats([
        -1.0, 1.0, 0.0, 0.0,
        1.0, 1.0, 1.0, 0.0,
        -1.0, -1.0, 0.0, 1.0,
        1.0, -1.0, 1.0, 1.0 
    ]))
    ib = IndexBuffer.new(6, pack_ints([
        0, 1, 2, 1, 3, 2
    ]))
    context.vbuffer(vb).ibuffer(ib).resource(Shader::PIXEL, 0, tex)

    timer = FPSTimer.new(120)
    mainloop {
        context.clear
        context.draw_index(TOPOLOGY_TRIANGLES, 0, 6)
        context.render 
        timer.wait
        swap_buffers
        
    }
}