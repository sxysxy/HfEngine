#encoding: utf-8
require 'HFSF'
CURRENT_PATH = File.dirname(__FILE__)
KOISHI_FILENAME = File.join(CURRENT_PATH, "300px-Komeiji Koishi.jpg")
CIRNO_FILENAME = File.join(CURRENT_PATH, "300px-Cirno.jpg")
YUYUKO_FILENAME = File.join(CURRENT_PATH, "300px-Yuyuko.jpg")
FLANDRE_FILENAME = File.join(CURRENT_PATH, "500px-Flandre.jpg")
MARISA_FILENAME = File.join(CURRENT_PATH, "700px-Marisa.jpg")
SAKUYA_FILENAME = File.join(CURRENT_PATH, "1000px-Sakuya.jpg")

HEG::GDevice.create
HEG::Window.new("Very Simple Texture Mapping Demo", 600, 600).instance_exec {
    show
    handle(:closed) { break_loop }
    context = HEG::RenderContext.new.viewport(0, 0, width(), height()).target(canvas())
    sf = HFSF.loadsf_code {
        Program("Draw") {
            Code(%{
                struct vsout {
                    float4 position : SV_POSITION;
                    unsigned int texid : ID;
                    float2 texcoord : TEX;
                };
                cbuffer transform : register(b0) {
                    float4x4 trans;
                }
            
                vsout VS(float3 pos : POS, unsigned int id : ID, float2 tex : TEX) {
                    vsout opt;
                    opt.position = mul(float4(pos, 1.0), trans);
                    opt.texcoord = tex;
                    opt.texid = id;
                    return opt;
                }

                Texture2D koishi : register(t0);
                Texture2D cirno : register(t1);
                Texture2D yuyuko : register(t2);
                Texture2D flandre : register(t3);
                Texture2D marisa : register(t4);
                Texture2D sakuya : register(t5);

                SamplerState color_sampler : register(s0);

                float4 PS(vsout vo) : SV_TARGET {
                    if(vo.texid == 0) {
                        return koishi.Sample(color_sampler, vo.texcoord);
                    }else if(vo.texid == 1) {
                        return cirno.Sample(color_sampler, vo.texcoord);
                    }else if(vo.texid == 2) {
                        return yuyuko.Sample(color_sampler, vo.texcoord);
                    }else if(vo.texid == 3) {
                        return flandre.Sample(color_sampler, vo.texcoord);
                    }else if(vo.texid == 4) {
                        return marisa.Sample(color_sampler, vo.texcoord);
                    }else if(vo.texid == 5) {
                        return sakuya.Sample(color_sampler, vo.texcoord);
                    }else {
                        return float4(0.0, 0.0, 0.0, 0.0);
                    }
                }
            })
            InputLayout {
                format "POS", HEG::FORMAT_FLOAT3
                format "ID", HEG::FORMAT_UINT
                format "TEX", HEG::FORMAT_FLOAT2
            }
            Resource {
                Sampler("sampler") {}
                ConstantBuffer("transform") {
                    size 64
                }
            }

            Section("draw") {
                vshader("VS")
                pshader("PS")
                vs_cbuffer(0, "transform")
                ps_sampler(0, "sampler")
            }
        }
    }
    sf.section[:draw].apply(context)
    sf.layout.apply(context)
    vb = HEG::VertexBuffer.new(24, 4 * 6, [
        #font
        [-1.0, 1.0, -1.0,   0,   0.0, 0.0],
        [1.0, 1.0, -1.0,    0,   1.0, 0.0],
        [-1.0, -1.0, -1.0,  0,   0.0, 1.0],
        [1.0, -1.0, -1.0,   0,   1.0, 1.0],

        #back
        [-1.0, 1.0, 1.0,    1,   0.0, 0.0],
        [1.0, 1.0, 1.0,     1,   1.0, 0.0],
        [-1.0, -1.0, 1.0,   1,   0.0, 1.0],
        [1.0, -1.0, 1.0,    1,   1.0, 1.0],

        #top
        [-1.0, 1.0, 1.0,    2,   0.0, 0.0],
        [1.0, 1.0, 1.0,     2,   1.0, 0.0],
        [-1.0, 1.0, -1.0,   2,   0.0, 1.0],
        [1.0, 1.0, -1.0,    2,   1.0, 1.0],

        #bottom
        [-1.0, -1.0, 1.0,   3,   0.0, 0.0],
        [1.0, -1.0, 1.0,    3,   1.0, 0.0],
        [-1.0, -1.0, -1.0,  3,   0.0, 1.0],
        [1.0, -1.0, -1.0,   3,   1.0, 1.0],

        #left
        [-1.0, 1.0, -1.0,    4,  0.0, 0.0],
        [-1.0, 1.0, 1.0,     4,  1.0, 0.0],
        [-1.0, -1.0, -1.0,   4,  0.0, 1.0],
        [-1.0, -1.0, 1.0,    4,  1.0, 1.0],

        #right
        [1.0, 1.0, -1.0,     5,  0.0, 0.0],
        [1.0, 1.0, 1.0,      5,  1.0, 0.0],
        [1.0, -1.0, -1.0,    5,  0.0, 1.0],
        [1.0, -1.0, 1.0,     5,  1.0, 1.0],
    ].flatten.pack("fffiff" * 24))
    ib = HEG::IndexBuffer.new(36, pack_ints([
        [0, 1, 2, 1, 3, 2],
        [4, 5, 6, 5, 7, 6],
        [8, 9, 10, 9, 11, 10],
        [12, 13, 14, 13, 15, 14],
        [16, 17, 18, 17, 19, 18],
        [20, 21, 22, 21, 23, 22]
    ].flatten))

    koishi = HEG::Canvas.new(KOISHI_FILENAME)
    cirno = HEG::Canvas.new(CIRNO_FILENAME)
    yuyuko = HEG::Canvas.new(YUYUKO_FILENAME)
    flandre = HEG::Canvas.new(FLANDRE_FILENAME)
    marisa = HEG::Canvas.new(MARISA_FILENAME)
    sakuya = HEG::Canvas.new(SAKUYA_FILENAME)

    context.vbuffer(vb).ibuffer(ib).
                resource(HEG::Shader::PIXEL, 0, koishi).resource(HEG::Shader::PIXEL, 1, cirno).
                resource(HEG::Shader::PIXEL, 2, yuyuko).resource(HEG::Shader::PIXEL, 3, flandre).
                resource(HEG::Shader::PIXEL, 4, marisa).resource(HEG::Shader::PIXEL, 5, sakuya)
    
    timer = HEG::FPSTimer.new(HEG::GDevice.instance.monitor_info[:refresh_rate])
    tick = 0
    mainloop {
        context.clear
        trans = HEG::Transform.new.rotate([0.5, 1.0, 0.3], (tick % 360) * Math::PI / 180.0).
            view([0.0, 1.5, -5.0], [0.0, 0.0, 0.0],[0.0, 1.0, 0.0]).    
            perspective(45 * Math::PI / 180.0, 1.0, 1.0, 100.0)
        context.flush(sf.resource.cbuffer[:transform], trans.data)
        context.draw_index(HEG::TOPOLOGY_TRIANGLES, 0, 36)
        context.render

        timer.wait
        tick += 1
        swap_buffers
    }
    [koishi, cirno, yuyuko, flandre, marisa, sakuya, vb, ib, context, sf, HEG::GDevice.instance].each {|x| x.release}
}