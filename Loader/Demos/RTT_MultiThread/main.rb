#When multi-thread(using a RemoteRenderExecutive, check whether it works properly)
require 'libcore'
include DX
SHADER_FILE = File.join(File.dirname(__FILE__), "Shaders.rb")

HFWindow.new("RTT", 500, 500) {
	show
	#set_async_move true
	set_handler(:on_closed) {exit_mainloop}
	device = D3DDevice.new
	rp = RenderPipelineM.new(device)
	sf = HFSF::loadsf_file(device, SHADER_FILE)[0]
	sf.section[:set].apply(rp)
	sf.input_layout.apply(rp)
	swapchain = SwapChain.new(device, self)
	rp.set_topology(TOPOLOGY_TRIANGLESTRIP).set_viewport(HFRect(0, 0, width, height))
	vecs = [-0.5, 0.5, 1.0, 0.0, 0.0, 1.0,
			0.5, 0.5, 0.0, 1.0, 1.0, 1.0, 
			-0.5, -0.5, 0.0, 1.0, 0.0, 1.0,
			0.5, -0.5, 1.0, 0.0, 1.0, 1.0].pack("f*")
	vb = VertexBuffer.new(device, 6*4, 4, vecs)
	vecs2 = [-1.0, 1.0, 0.0, 0.0, 0.0, 0.0,
			1.0, 1.0, 1.0, 0.0, 0.0, 0.0, 
			-1.0, -1.0, 0.0, 1.0, 0.0, 0.0,
			1.0, -1.0, 1.0, 1.0, 0.0, 0.0].pack("f*")
	vb2 = VertexBuffer.new(device, 6*4, 4, vecs2)
	
	re = RemoteRenderExecutive.new(device, swapchain, 60)
	re.insert(rp, 100)
	
	#RTT
	tex = Texture2D.new(device, width, height)
	rtt = RTT.new(tex)
		
	timer = FPSTimer.new(60)
	messageloop {
		re.lock #purposely in the loop
		sf.section[:draw_shape].apply(rp)
		rp.set_target(rtt)
		rp.clear(HFColorRGBA(1.0, 1.0, 1.0, 1.0))
		rp.set_vbuffer(vb)
		rp.draw(0, 4)
		rp.immdiate_render
		re.unlock

		sf.section[:draw_texture].apply(rp)
		rp.set_target(swapchain.rtt)
		rp.set_vbuffer(vb2)
		rp.clear(HFColorRGBA(0.0, 0.0, 0.0, 0.0))
		rp.set_ps_resource(0, tex)
		rp.draw(0, 4)
		rp.set_ps_resource(0, nil)
		
		rp.swap_commands
		timer.await
	}
	re.terminate
	[rp, re, swapchain, vb, vb2, sf, tex, rtt, device].each &:release
}