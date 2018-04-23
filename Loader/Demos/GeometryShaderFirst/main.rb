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
    vecs = [0.0, 0.0, 0.0].flatten.pack("f*")
	vb = VertexBuffer.new(device, 4*3, 1, vecs)
	rp.set_vbuffer(vb).set_target(swapchain.rtt).\
		set_viewport(HFRect(0, 0, width, height)).set_topology(TOPOLOGY_POINTLIST)
	draw_triangle = ->(edge_len, center_pos, color){
		rp.update_subresource sf.resource.cbuffer[:GSParam], [edge_len.to_f, 0.0, 0.0, 0.0].pack("f*")
		rp.update_subresource sf.resource.cbuffer[:PSParam], color.row_data_ptr
		rp.update_subresource vb, [center_pos[0].to_f, center_pos[1].to_f, 0.0].pack("f*")
		rp.draw(0, 1)
	}
	timer = FPSTimer.new(60)
	messageloop {
		rp.clear(HFColorRGBA(0.0, 0.0, 0.0, 0.0))
		draw_triangle[0.5, [0.0, 0.0], HFColorRGBA(1.0, 0.0, 1.0, 1.0)]
		draw_triangle[0.4, [0.4, -0.3], HFColorRGBA(0.0, 1.0, 0.0, 1.0)]
		draw_triangle[0.6, [-0.5, 0.6], HFColorRGBA(1.0, 1.0, 0.0, 1.0)]
		draw_triangle[0.5, [-0.4, -0.2], HFColorRGBA(0.0, 1.0, 1.0, 1.0)]
		draw_triangle[0.33, [-0.2, 0.2], HFColorRGBA(1.0, 0.0, 0.0, 1.0)]
		draw_triangle[0.7, [-0.6, -0.7], HFColorRGBA(1.0, 1.0, 1.0, 1.0)]
		rp.immdiate_render
		swapchain.present
		timer.await
	}
}