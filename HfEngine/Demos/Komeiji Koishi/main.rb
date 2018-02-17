#encoding: utf-8
require './libcore.rb'
HFWindow.new("恋恋 VS 紫妈", 300, 300) {   
    show
    window = self
    set_handler(:on_closed) {exit_mainloop}
    device = DX::D3DDevice.new(DX::HARDWARE_DEVICE);
    swap_chain = DX::SwapChain.new(device, window)
    context = DX::D3DDeviceContext.new(device).bind_pipeline(DX::RenderPipeline.new {
        set_vshader DX::Shader.load_hlsl(device, "texture_vs.shader", DX::VertexShader)
        set_pshader DX::Shader.load_hlsl(device, "texture_ps.shader", DX::PixelShader)
        set_input_layout device, ["POSITION", "TEXCOORD"], [DX::R32G32_FLOAT, DX::R32G32_FLOAT]
    }).instance_eval {
        koishi = DX::D3DTexture2D.new(device, "../CommonFiles/300px-Komeiji Koishi.jpg")
		yukari = DX::D3DTexture2D.new(device, "../CommonFiles/250px-Yukari.jpg")
        vecs = [[-1.0, -1.0], [0.0, 1.0],
                [-1.0, 1.0],  [0.0, 0.0],
                [1.0, -1.0],  [1.0, 1.0],
                [1.0, 1.0],   [1.0, 0.0]].flatten.pack("f*")
        bind_vbuffer(0, DX::D3DVertexBuffer.new(device, vecs.size, vecs), 4*4)
        bind_resources(0, [koishi, yukari], DX::SHADERS_APPLYTO_PSHADER)
		bind_sampler(0, DX::D3DSampler.new{
			use_default
			create_state(device)
		}, DX::SHADERS_APPLYTO_PSHADER)
        set_topology(DX::TOPOLOGY_TRIANGLESTRIP)
        set_viewport(HFRect.new(0, 0, window.width, window.height), 0.0, 1.0)
        set_render_target(swap_chain.back_buffer)
        self
    }
	param = [1.0, 0.0, 0.0, 0.0].pack("f*")
	pcbuffer = DX::D3DConstantBuffer.new(device, param.size, param)
	context.bind_cbuffer(0, pcbuffer, DX::SHADERS_APPLYTO_PSHADER)
	rth = DX::RenderingThread.new(device, swap_chain, 60)
	keyboard = DX::Input::Keyboard.new(window)
	timer = FPSTimer.new(60)
    messageloop {
		keyboard.update
		exit_mainloop if keyboard.is_pressed_now(DX::DIK_ESC)
		move_to(get_position[0]-10, get_position[1]) if keyboard.is_pressed_now(DX::DIK_A) 
		move_to(get_position[0]+10, get_position[1]) if keyboard.is_pressed_now(DX::DIK_D) 
		move_to(get_position[0], get_position[1]+10) if keyboard.is_pressed_now(DX::DIK_S)
		move_to(get_position[0], get_position[1]-10) if keyboard.is_pressed_now(DX::DIK_W)
		$rate ||= 1.0
		if keyboard.is_pressed_now(DX::DIK_UP)
			$rate += 0.01 if $rate < 1.0
			context.update_subresource pcbuffer, [$rate, 0.0, 0.0, 0.0].pack("f*")
		elsif keyboard.is_pressed_now(DX::DIK_DOWN)
			$rate -= 0.01 if $rate > 0.0
			context.update_subresource pcbuffer, [$rate, 0.0, 0.0, 0.0].pack("f*")
		end
        context.clear_render_target(swap_chain.back_buffer, HFColorRGBA.new(0.0, 0.0, 0.0, 0.0));
        context.draw(0, 4)
		context.finish_command_list
		rth.push_command_list context
		timer.await
    }
	rth.terminate
}
