#encoding: utf-8
require 'libcore.rb'
include DX
CURRENT_PATH = File.dirname(__FILE__)
SHADER_FILENAME = File.join(CURRENT_PATH, "shaders.rb")
KOISHI_FILENAME = File.join(CURRENT_PATH, "300px-Komeiji Koishi.jpg")
YUKARI_FILENAME = File.join(CURRENT_PATH, "250px-Yukari.jpg")

HFWindow.new("恋恋 VS 紫妈", 300, 300) {   
    show
    set_handler(:on_closed) {exit_mainloop}
    device = D3DDevice.new
    swapchain = SwapChain.new(device, self)
	vecs = [[-1.0, -1.0], [0.0, 1.0],
            [-1.0, 1.0],  [0.0, 0.0],
            [1.0, -1.0],  [1.0, 1.0],
            [1.0, 1.0],   [1.0, 0.0]].flatten.pack("f*")
	vb = VertexBuffer.new(device, 4*4, 4, vecs)
	rp = RenderPipelineM.new(device).set_topology(TOPOLOGY_TRIANGLESTRIP)\
	 .set_target(swapchain.rtt).set_viewport(HFRect(0, 0, width, height))\
						.set_vbuffer(vb)
	sf = HFSF::loadsf_file(device, SHADER_FILENAME)[0]
	sf.section[:set].apply(rp) 
	sf.input_layout.apply(rp)  
	pcbuffer = sf.resource.cbuffer[:param]  	
	
	koishi = Texture2D.new(device, KOISHI_FILENAME)
	yukari = Texture2D.new(device, YUKARI_FILENAME)					
    rp.set_ps_resource(0, koishi).set_ps_resource(1, yukari)
	re = RemoteRenderExecutive.new(device, swapchain, 60)
	re.insert(rp, 100)
	keyboard = Input::Keyboard.new(self)
	timer = FPSTimer.new(60)
    messageloop {
		keyboard.update
		exit_mainloop if keyboard.is_pressed_now(DX::DIK_ESC)
		move_to(get_position[0]-10, get_position[1]) if keyboard.is_pressed_now(DX::DIK_A) 
		move_to(get_position[0]+10, get_position[1]) if keyboard.is_pressed_now(DX::DIK_D) 
		move_to(get_position[0], get_position[1]+10) if keyboard.is_pressed_now(DX::DIK_S)
		move_to(get_position[0], get_position[1]-10) if keyboard.is_pressed_now(DX::DIK_W)
		$rate ||= 0.5
		if keyboard.is_pressed_now(DX::DIK_UP)
			$rate += 0.01 if $rate < 1.0
			rp.update_subresource pcbuffer, [$rate, 0.0, 0.0, 0.0].pack("f*")
		elsif keyboard.is_pressed_now(DX::DIK_DOWN)
			$rate -= 0.01 if $rate > 0.0
			rp.update_subresource pcbuffer, [$rate, 0.0, 0.0, 0.0].pack("f*")
		end
        rp.clear(HFColorRGBA(0.0, 0.0, 0.0, 0.0))
        rp.draw(0, 4)
		rp.swap_commands
		timer.await
    }
	re.terminate
	[swapchain, vb, sf, keyboard, koishi, yukari, re, rp, device].each &:release
}
