#encoding: utf-8
require './libcore.rb'
HFWindow.new("Demo", 500, 500) { 
    show
    window = self
    set_handler(:on_closed) {exit_mainloop}
    device = DX::D3DDevice.new(DX::HARDWARE_DEVICE);
    swap_chain = DX::SwapChain.new(device, window)
    context = device.immcontext.bind_pipeline(DX::RenderPipeline.new {
        set_vshader DX::VertexShader.load_string(device, RSDSL.generate {
			struct(:vs_output) {
				declare :pos, :float4, :SV_POSITION
				declare :color, :float4, :COLOR
			}
			defunc(:main, :vs_output, [:pos, :float4, :POSITION], [:color, :float4, :COLOR]) {
				dvar :opt, :vs_output
				p "opt.color = color, opt.pos = pos;"
				creturn {p :opt}
			}
		})
        set_pshader DX::PixelShader.load_string(device, RSDSL.generate {
			defunc(:main, :float4, [:pos, :float4, :SV_POSITION], [:color, :float4, :COLOR], :SV_TARGET) {
				creturn {p "color"}
			}
		})
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
	timer = FPSTimer.new(30)
    messageloop {
        context.clear_render_target(swap_chain.back_buffer, HFColorRGBA.new(0.0, 0.0, 0.0, 0.0));
        context.draw(0, 4)
		swap_chain.present
        timer.await
    }
}

