#encoding : utf-8
require './libcore.rb'
include DX
HFWindow.new("喵喵喵？", 600, 600) {
    show
    set_handler(:on_closed) {exit_mainloop}
    device = D3DDevice.new(HARDWARE_DEVICE)
    swap_chain = SwapChain.new(device, self)
    context = device.immcontext
    context.bind_pipeline(RenderPipeline.new {
        set_vshader Shader.load_hlsl(device, "VS.shader", VertexShader)
        set_pshader Shader.load_hlsl(device, "PS.shader", PixelShader)
        set_input_layout device, ["POSITION"], [R32G32_FLOAT]
    })
    context.set_topology(TOPOLOGY_TRIANGLESTRIP)
    vecs = [-0.5, -0.5,
           -0.5, 0.5,  
           0.5, -0.5, 
           0.5, 0.5].pack("f*")
    context.bind_vbuffer(0, D3DVertexBuffer.new(device, vecs.size, vecs), 2*4)
    color = [0.0, 1.0, 1.0, 1.0].pack("f*")
    context.bind_cbuffer(0, D3DConstantBuffer.new(device, color.size, color), SHADERS_APPLYTO_PSHADER)
    context.set_render_target(swap_chain.backbuffer)
    context.set_viewport(HFRect.new(0, 0, width, height), 0.0, 1.0)
    
    keyboard = Input::Keyboard.new(self)
    timer = FPSTimer.new(60)
    messageloop{
        keyboard.update
        exit_mainloop if keyboard.is_pressed_now(DX::DIK_ESC)  #按ESC退出
        
        context.draw(0, 4)
        swap_chain.present
        timer.await
    }
}