#encoding :utf-8
require 'libcore'
include DX
SHADER_FILE = File.join(File.dirname(__FILE__), "Shaders.rb")
PICTURE_FILE = File.join(File.dirname(__FILE__), "point.png")
show_console
HFWindow.new("Create Texture", 400, 400) {
	show
	set_handler(:on_closed) {exit_mainloop}
	
	device = D3DDevice.new
	swapchain = SwapChain.new(device, self)
	rp = RenderPipeline.new(device)
	sf = HFSF::loadsf_file(device, SHADER_FILE)[0]
	sf.section[:set].apply(rp)
	sf.input_layout.apply(rp)
	vecs = [-1.0, 1.0, 0.0, 0.0,
			1.0, 1.0, 1.0, 0.0,
			-1.0, -1.0, 0.0, 1.0,
			1.0, -1.0, 1.0, 1.0].pack("f*")
	vb = VertexBuffer.new(device, 16, 4, vecs)
	rp.set_target(swapchain.rtt).set_vbuffer(vb).set_topology(TOPOLOGY_TRIANGLESTRIP)
	
	#Create from row data(RGBA array)
	t1 = Texture2D.new(device, 100, 100, [0, 0, 255, 255].pack("C*")*10000)
	
	#Create from file
	t2 = Texture2D.new(device, PICTURE_FILE)
	#data = rp.immdiate_dump_pixels2d(t2)
	#print data.unpack("C*")
	#STDOUT.flush
	
	messageloop {
		rp.set_viewport(HFRect(0, 0, height, width))
		rp.clear(HFColorRGBA(0.0, 0.0, 0.0, 0.0))
		rp.set_viewport(HFRect(0, 0, 100, 100))
		rp.set_ps_resource(0, t1)
		rp.draw(0, 4)
		rp.set_ps_resource(0, nil)
		
		rp.set_viewport(HFRect(100, 100, 300, 300))
		rp.set_ps_resource(0, t2)
		rp.draw(0, 4)
		rp.set_ps_resource(0, nil)
		
		rp.immdiate_render
		swapchain.present
	}
	
}