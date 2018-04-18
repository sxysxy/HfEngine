#encoding : utf-8
require 'libcore.rb'
include DX
CURRENT_PATH = File.dirname(__FILE__)
SHADER_FILENAME = File.join(CURRENT_PATH, "shaders.shader")
HFWindow.new("Demo", 500, 500) { 
    show
	set_handler(:on_closed) {exit_mainloop}
    device = D3DDevice.new(HARDWARE_DEVICE)
	swapchain = SwapChain.new(device, self)
	vs = VertexShader.load_hlsl(device, SHADER_FILENAME, "VS")
	ps = PixelShader.load_hlsl(device, SHADER_FILENAME, "PS")
    vecs = [[-1.0, -1.0, -1.0], [1.0, 1.0, 1.0, 1.0],
			[-1.0, +1.0, -1.0], [0.0, 0.0, 0.0, 1.0],
			[+1.0, +1.0, -1.0], [1.0, 0.0, 0.0, 1.0],
			[+1.0, -1.0, -1.0], [1.0, 1.0, 0.0, 1.0],
			[-1.0, -1.0, +1.0], [0.0, 0.0, 1.0, 1.0],
			[-1.0, +1.0, +1.0], [1.0, 1.0, 0.0, 1.0],
			[+1.0, +1.0, +1.0], [0.0, 1.0, 1.0, 1.0],
			[+1.0, -1.0, +1.0], [1.0, 0.0, 1.0, 1.0]].flatten.pack("f*")
	vb = VertexBuffer.new(device, 7*4, 8, vecs)
	indexs = [0,1,2,0,2,3,
			  4,6,5,4,7,6,
			  4,5,1,4,1,0,
			  3,2,6,3,6,7,
			  1,5,6,1,6,2,
			  4,0,3,4,3,7].pack("i*")
	ib = IndexBuffer.new(device, 36, indexs)
	cb = ConstantBuffer.new(device, 64)
	rp = RenderPipeline.new(device).set_vshader(vs).set_pshader(ps).set_topology(TOPOLOGY_TRIANGLELIST)\
	 .set_target(swapchain.rtt).set_input_layout(device, ["POSITION", "COLOR"], 
						[R32G32B32_FLOAT, R32G32B32A32_FLOAT]).set_viewport(HFRect(0, 0, width, height))\
						.set_vbuffer(vb).set_ibuffer(ib).set_vs_cbuffer(0, cb)
	timer = FPSTimer.new(60);
	t = 0
	w, v, p, wvp = Array.new(4) {MathTool::Matrix4x4.new}
	messageloop{
		rp.clear(HFColorRGBA(0.0, 0.0, 0.0, 0.0))
		t += 1
		w.rotate_round!([1.0, 1.0, 1.0, 0.0], t * 0.0125) 
		v.lookat!([-5.0, 2.0, 2.0, 0.0], [0.0, 0.0, 0.0, 0.0])
		p.perspective!(MathTool::PIDIV4, 1.0, 0.01, 100.0)
		wvp = w*v*p #also, you can use MathTool.rotate_round... but it will generate a new object
		rp.update_subresource(cb, wvp.tranpose!.row_data_ptr);
		rp.draw_index(0, 36)
		rp.immdiate_render
		swapchain.present
		timer.await
	}
}
