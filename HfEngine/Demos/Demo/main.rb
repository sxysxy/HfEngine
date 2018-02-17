#encoding: utf-8
require './libcore.rb'
HFWindow.new("Demo", 500, 500) { 
    set_fixed true
    show
    window = self
    set_handler(:on_closed) {exit_mainloop}
    device = DX::D3DDevice.new(DX::HARDWARE_DEVICE);
    swap_chain = DX::SwapChain.new(device, window)
    context = device.immcontext.bind_pipeline(DX::RenderPipeline.new {
        set_vshader DX::Shader.load_hlsl(device, "render_vs.shader", DX::VertexShader)
        set_pshader DX::Shader.load_hlsl(device, "render_ps.shader", DX::PixelShader)
        set_input_layout device, ["POSITION", "COLOR"], [DX::R32G32B32_FLOAT, DX::R32G32B32A32_FLOAT]
    }).instance_eval {
        vecs = [[-0.5, -0.5, 0.0], [0.0, 1.0, 0.0, 1.0],
                [-0.5, 0.5, 0.0],  [1.0, 0.0, 1.0, 1.0],
                [0.5, -0.5, 0.0],  [0.0, 0.0, 1.0, 1.0],
                [0.5, 0.5, 0.0],   [0.0, 1.0, 1.0, 1.0]].flatten.pack("f*")
        bind_vbuffer(0, DX::D3DVertexBuffer.new(device, vecs.size, vecs), 4*7)
        set_topology(DX::TOPOLOGY_TRIANGLESTRIP)
        set_viewport(HFRect.new(0, 0, window.width, window.height), 0.0, 1.0)
        set_render_target(swap_chain.back_buffer)
        self
    }
    messageloop {
        context.clear_render_target(swap_chain.back_buffer, HFColorRGBA.new(0.0, 0.0, 0.0, 0.0));
        context.draw(0, 4)
        swap_chain.present(DX::SwapChain::VSYNC_2_BLANK);
    }
}

