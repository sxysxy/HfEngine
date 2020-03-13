#encoding: utf-8
require 'EasyTTF'
require 'HFSF'
include HEG
GDevice.create
show_console
Window.new("Simple Font Rendering Demo", 400, 400).instance_exec {
    show 
    handle(:closed) { break_loop }
    
    font = TTF::Font.new("msyh.ttc")
    board = TTF::Blackboard.new(400, 400)
    board.write("abcdefghijklmnnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ", font, Bitmap.color(255, 255, 255, 255))
    board.write("\n0123456789\n", font, Bitmap.color(0, 0, 255, 255))
    board.write("零一二三四五六七八九十九八七六五四三二一零\n", font, Bitmap.color(255, 255, 0, 255))
    board.write("你\n  我\n   它\n", font, Bitmap.color(255, 0, 0, 255))
    board.write("  手持两把锟斤拷，口中疾呼烫烫烫。\n  脚踏千朵屯屯屯，笑看万物锘锘锘。", font, Bitmap.color(255, 255, 255, 255))
    tex = board.to_canvas
    font.release
    board.release
    context = RenderContext.new.viewport(0, 0, width(), height()).target(self.canvas())
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