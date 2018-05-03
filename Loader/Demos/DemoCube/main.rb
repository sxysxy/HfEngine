#encoding : utf-8
require 'libcore.rb'
include DX
CURRENT_PATH = File.dirname(__FILE__)
SHADER_FILENAME = File.join(CURRENT_PATH, "shaders.rb")
FPS = 144
HFWindow.new("Demo", 500, 500) { 
    show
	set_handler(:on_closed) {exit_mainloop}
    device = D3DDevice.new
	swapchain = SwapChain.new(device, self)
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
	rp = RenderPipeline.new(device).set_topology(TOPOLOGY_TRIANGLELIST)\
	 .set_target(swapchain.rtt).set_viewport(HFRect(0, 0, width, height))\
						.set_vbuffer(vb).set_ibuffer(ib)
	sf = HFSF::loadsf_file(device, SHADER_FILENAME)[0]
	sf.section[:set].apply(rp) #一键设置一组shader（以及需要的constant buffer等资源
	sf.input_layout.apply(rp)  #设置input_layout
	cb = sf.resource.cbuffer[:wvpmatrix]  #得到资源段内名为 wvpmatrix 的constant buffer, 因为需要更新
	re = RemoteRenderExecutive.new(device, swapchain, FPS)
	timer = FPSTimer.new(FPS)
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
		re.push(rp)
		timer.await
	}
	re.terminate
	[swapchain, vb, ib, rp, sf, re, device].each &:release
}
