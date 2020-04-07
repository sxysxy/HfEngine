#encoding: utf-8
require 'HFSF'
require 'Graphics3D'
require 'Extends'
CURRENT_PATH = File.dirname(__FILE__)
KOISHI_FILENAME = File.join(CURRENT_PATH, "300px-Komeiji Koishi.jpg")
CIRNO_FILENAME = File.join(CURRENT_PATH, "300px-Cirno.jpg")
YUYUKO_FILENAME = File.join(CURRENT_PATH, "300px-Yuyuko.jpg")
FLANDRE_FILENAME = File.join(CURRENT_PATH, "500px-Flandre.jpg")
MARISA_FILENAME = File.join(CURRENT_PATH, "700px-Marisa.jpg")
SAKUYA_FILENAME = File.join(CURRENT_PATH, "1000px-Sakuya.jpg")
MESH_DENSITY = 90
CAMERA_MOVE_SPEED = 0.04
CAMERA_ANGLE_SPEED = 1.0

HEG::GDevice.create
HEG::Window.new("Very Simple Texture Mapping Demo", 600, 600).instance_exec {
    show
    handle(:closed) { break_loop }
    context = HEG::RenderContext.new.viewport(0, 0, width(), height()).target(canvas())

    #--------------------------cube----------------------------------------------------
    cube_sf = HFSF.loadsf_code {
        Program("DrawCube") {
            Code(%{
                struct vsout {
                    float4 position : SV_POSITION;
                    unsigned int texid : ID;
                    float2 texcoord : TEX;
                };
                cbuffer W : register(b0) {
                    float4x4 world; 
                }
                cbuffer VP : register(b1) {
                    float4x4 viewproj;
                }
            
                vsout VS(float3 pos : POS, unsigned int id : ID, float2 tex : TEX) {
                    vsout opt;
                    opt.position = mul(mul(float4(pos, 1.0), world), viewproj);
                   
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
            }

            Section("draw") {
                vshader("VS")
                pshader("PS")
                ps_sampler(0, "sampler")
            }
        }
    }
    cube_vb = HEG::VertexBuffer.new(24, 4 * 6, [
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
    cube_ib = HEG::IndexBuffer.new(36, pack_ints([
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

    #----------------------------------mesh---------------------------------
    mesh_sf = HFSF.loadsf_code {
        Program("DrawMesh") {
            Code(%{
                cbuffer W : register(b0) {
                    float4x4 world; 
                }
                cbuffer VP : register(b1) {
                    float4x4 viewproj;
                }
                float4 VS(float3 pos : POS) : SV_POSITION {
                    return mul(mul(float4(pos, 1.0), world), viewproj);
                }
                float4 PS(float4 pos : SV_POSITION) : SV_TARGET{
                    return float4(1.0, 1.0, 1.0, 1.0);
                }
            })
            InputLayout {
                format "POS", HEG::FORMAT_FLOAT3
            }
            Section("draw") {
                vshader("VS")
                pshader("PS")
            }
        }
    }

    mesh_vs = []
    (0..MESH_DENSITY).each {|i|
        mesh_vs.push([-1.0, 0.0, -1.0 + 2.0 * i / MESH_DENSITY])
        mesh_vs.push([ 1.0, 0.0, -1.0 + 2.0 * i / MESH_DENSITY])
    }
    (0..MESH_DENSITY).each {|i|
        mesh_vs.push([-1.0 + 2.0 * i / MESH_DENSITY, 0.0, -1.0])
        mesh_vs.push([-1.0 + 2.0 * i / MESH_DENSITY, 0.0,  1.0])
    }
    mesh_vb = HEG::VertexBuffer.new(mesh_vs.size, 3 * 4, pack_floats(mesh_vs.flatten))

    context.resource(HEG::Shader::PIXEL, 0, koishi).resource(HEG::Shader::PIXEL, 1, cirno).
            resource(HEG::Shader::PIXEL, 2, yuyuko).resource(HEG::Shader::PIXEL, 3, flandre).
            resource(HEG::Shader::PIXEL, 4, marisa).resource(HEG::Shader::PIXEL, 5, sakuya)
    
    timer = HEG::FPSTimer.new(120)
    camera = HEG::Camera.new
    world_buffer = HEG::ConstantBuffer.new(64)
    viewproj_buffer = HEG::ConstantBuffer.new(64)
    fov = 45.0

    tick = 0
    mainloop {
        #update key
        update_key
        #update camera:
        camera.move_forward(CAMERA_MOVE_SPEED) if is_key_pressed(HEG::KEY_W)
        camera.move_backward(CAMERA_MOVE_SPEED) if is_key_pressed(HEG::KEY_S)
        camera.move_left(CAMERA_MOVE_SPEED) if is_key_pressed(HEG::KEY_A)
        camera.move_right(CAMERA_MOVE_SPEED) if is_key_pressed(HEG::KEY_D)
        camera.move_up(CAMERA_MOVE_SPEED) if is_key_pressed(HEG::KEY_E)
        camera.move_down(CAMERA_MOVE_SPEED) if is_key_pressed(HEG::KEY_C)
        camera.yaw(-CAMERA_ANGLE_SPEED) if is_key_pressed(HEG::KEY_J)
        camera.yaw(CAMERA_ANGLE_SPEED) if is_key_pressed(HEG::KEY_L)
        camera.pitch(CAMERA_ANGLE_SPEED) if is_key_pressed(HEG::KEY_I)
        camera.pitch(-CAMERA_ANGLE_SPEED) if is_key_pressed(HEG::KEY_K)
        if is_key_pressed(HEG::KEY_U)
            fov += 0.5
            fov = 89.5 if fov >= 90.0
        end 
        if is_key_pressed(HEG::KEY_O) 
            fov -= 0.5
            fov = 0.5 if fov <= 0.5
        end
        if is_key_pressed(HEG::KEY_R)
            camera.reset 
            fov = 45.0
        end
        context.clear
        viewproj = HEG::Transform.new.view(camera.position, camera.target, camera.updir).perspective(fov * Math::PI / 180.0, 1.0, 0.1, 100.0)
        context.flush(viewproj_buffer, viewproj.data)
        
        #
        tick += 1
        tick = 0 if tick == 360
        angle_rad = tick * Math::PI / 180.0 / 2

        #Draw the cube
        cube_sf.section[:draw].apply(context)
        cube_sf.layout.apply(context)
        context.cbuffer(HEG::Shader::VERTEX, 0, world_buffer)
        context.cbuffer(HEG::Shader::VERTEX, 1, viewproj_buffer)
        context.vbuffer(cube_vb).ibuffer(cube_ib)
        cube_w = HEG::Transform.new.rotate([0.2, 0.8, 0.5], angle_rad).translate(2.0, 0.0, 0.0).rotate([0.0, 1.0, 0.0], angle_rad).translate(0.0, 0.0, 8.0)
        context.flush(world_buffer, cube_w.data)
        context.draw_index(HEG::TOPOLOGY_TRIANGLES, 0, 36)


        #Draw the Mesh
        mesh_sf.section[:draw].apply(context)
        mesh_sf.layout.apply(context)
        context.cbuffer(HEG::Shader::VERTEX, 0, world_buffer)
        context.cbuffer(HEG::Shader::VERTEX, 1, viewproj_buffer)
        context.vbuffer(mesh_vb)
        mesh_w = HEG::Transform.new.scale(16.0, 10.0, 10.0).translate(0.0, 2.0, 5.0)
        context.flush(world_buffer, mesh_w.data)
        context.draw(HEG::TOPOLOGY_LINES, 0, mesh_vs.size)
        mesh_w = HEG::Transform.new.scale(16.0, 10.0, 10.0).translate(0.0, -2.0, 5.0)
        context.flush(world_buffer, mesh_w.data)
        context.draw(HEG::TOPOLOGY_LINES, 0, mesh_vs.size)
        #Render
        context.render

        timer.wait
        tick += 1
        swap_buffers
    }
    [koishi, cirno, yuyuko, flandre, marisa, sakuya, cube_vb, cube_ib, mesh_vb, context, cube_sf, mesh_sf].each {|x| x.release}
}