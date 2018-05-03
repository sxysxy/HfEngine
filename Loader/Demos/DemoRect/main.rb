#encoding: utf-8
require 'libcore.rb'
include DX
CURRENT_PATH = File.dirname(__FILE__)
SHADER_FILENAME = File.join(CURRENT_PATH, "shaders.shader")
HFWindow.new("Demo", 500, 500) { 
    show
	set_handler(:on_closed) {exit_mainloop}
    device = D3DDevice.new
	swapchain = SwapChain.new(device, self)
	vs = VertexShader.load_hlsl(device, SHADER_FILENAME, "VS")
	ps = PixelShader.load_hlsl(device, SHADER_FILENAME, "PS")
	vecs = [[-0.5, -0.5, 0.0], [0.0, 1.0, 0.0, 1.0],
			[-0.5, 0.5, 0.0],  [0.0, 0.0, 1.0, 1.0],
			[0.5, -0.5, 0.0],  [1.0, 1.0, 0.0, 1.0],
			[0.5, 0.5, 0.0],   [1.0, 0.0, 1.0, 1.0]].flatten.pack("f*")
	vb = VertexBuffer.new(device, 7*4, 4, vecs)
	rp = RenderPipeline.new(device).set_vshader(vs).set_pshader(ps).set_topology(TOPOLOGY_TRIANGLESTRIP)\
	 .set_target(swapchain.rtt).set_input_layout(["POSITION", "COLOR"], 
						[R32G32B32_FLOAT, R32G32B32A32_FLOAT]).set_viewport(HFRect(0, 0, width, height))\
						.set_vbuffer(vb)
	
	timer = FPSTimer.new(60);
	messageloop{
		rp.clear(HFColorRGBA(1.0, 1.0, 1.0, 1.0))
		rp.draw(0, 4)
		rp.immdiate_render
		swapchain.present
		timer.await
	}
	[swapchain, vs, ps, vb, rp, device].each &:release
}

