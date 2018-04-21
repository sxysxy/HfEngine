require 'libcore'
include DX
SHADERS_FILENAME = File.join(File.dirname(__FILE__), "shaders.rb")
HFWindow.new("Geometry Shader Simple Demo", 500, 500) {
	show
	set_handler(:on_closed) {exit_mainloop}
	device = D3DDevice.new(HARDWARE_DEVICE)
	swapchain = SwapChain.new(device, self)
	rp = RenderPipeline.new(device)
	sf = HFSF::loadsf_file(device, SHADERS_FILENAME)[0]
	sf.section[:set].apply(rp)
	sf.input_layout.apply(rp)
	vecs = [0.0, 0.0, 0.0, 1,0, 0.0, 1.0, 1.0].pack("f*")
	vb = VertexBuffer.new(device, 4*7, 1, vecs)
	rp.set_vbuffer(vb).set_target(swapchain.rtt).\
		set_viewport(HFRect(0, 0, width, height)).set_topology(TOPOLOGY_POINTLIST)
	timer = FPSTimer.new(60)
	messageloop {
		rp.draw(0, 1)
		rp.immdiate_render
		swapchain.present
		timer.await
	}
}