#encoding: utf-8
require './libcore.rb'
HFWindow.new("Demo", 500, 500) { 
    show
    set_handler(:on_closed) {exit_mainloop}
    device = DX::D3DDevice.new(DX::HARDWARE_DEVICE)
    swap_chain = DX::SwapChain.new(device, self)
	renderer = G2D::Renderer.new(device, self)
	koishi = DX::D3DTexture2D.new(device, "./Demos/Komeiji Koishi/300px-Komeiji Koishi.jpg")
	yukari = DX::D3DTexture2D.new(device, "./Demos/Komeiji Koishi/250px-Yukari.jpg")
	back = DX::D3DTexture2D.new(device, width, height, true)
	ps = DX::PixelShader.load_string(device, RSDSL.generate {
		Texture2D(:color_map, 0)
		SamplerState(:color_sampler, 0)
		defunc(:main, :float4, [:pos, :float4, :SV_POSITION], [:tex, :float2, :TEXCOORD], :SV_TARGET) {
			dvar(:col, :float4) {p "color_map.Sample(color_sampler, tex)"}
			p "col.b += 0.4f;"
			creturn {p :col}
		}
	})
	timer = FPSTimer.new(233)
    messageloop {
		renderer.set_render_target(back)
		renderer.clear_target(HFColorRGBA(0.0, 0.0, 0.0, 0.0))
		renderer.z_depth = 0.5
		renderer.fill_rect(HFRect(100, 100, 200, 300), HFColorRGBA(0.0, 1.0, 0.0, 1.0))
		renderer.fill_rect(HFRect(300, 300, 50, 50), HFColorRGBA(1.0, 0.0, 1.0, 1.0))
		renderer.draw_texture(yukari, HFRect(350, 150, 150, 150))
		renderer.z_depth = 0.0
		renderer.draw_texture(koishi, HFRect(0, 0, 200, 200))
		renderer.execute_render
		renderer.set_render_target(swap_chain.back_buffer)
		renderer.clear_target(HFColorRGBA(0.0, 0.0, 0.0, 0.0))
		renderer.set_DT_PS(ps)
		renderer.draw_texture(back, HFRect(0, 0, width, height))
		renderer.use_default_DT_PS
		renderer.execute_render
		swap_chain.present
        timer.await
    }
}

