#encoding : utf-8
require 'libcore.rb'
include DX
CURRENT_PATH = File.dirname(__FILE__)
SHADER_FILENAME = File.join(CURRENT_PATH, "shaders.rb")
FPS = 233
HFWindow.new(%{OnePointPerspective}, 600, 600) {
    show
    set_handler(:on_closed) {exit_mainloop}
    device = D3DDevice.new
    swapchain = SwapChain.new(device, self)
    vecs = [[-1.0, -1.0, -1.0], [1.0, 0.0, 1.0, 1.0],
            [-1.0, +1.0, -1.0], [0.0, 1.0, 0.0, 1.0],
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
    sf.section[:set].apply(rp)
    sf.input_layout.apply(rp)
    cb = sf.resource.cbuffer[:wvpmatrix]
    timer = FPSTimer.new(FPS)
    scale = 100.0 / 600
    offset = 0.5 
    offsets = [[0.0,0.0],[0.0,offset],[offset,0.0],[offset,offset],
               [0.0,-offset],[-offset,0.0],[-offset, -offset],
               [offset, -offset], [-offset, offset]]
    messageloop {
        rp.clear(HFColorRGBA(0.0, 0.0, 0.0, 0.0))
        offsets.each do |x, y|
            world = MathTool::zoom(scale, scale, scale) * MathTool::move(x, y, 0.0)
            view = MathTool::lookat([0.0, 0.0, 1.0, 0.0], [0.0, 0.0, 0.0, 0.0])
            proj = MathTool::perspective(MathTool::PIDIV2, 1.0, 0.01, 100.0)
            rp.update_subresource(cb, (world*view*proj).tranpose!.row_data_ptr)
            rp.draw_index(0, 36)
        end
        rp.immdiate_render
        swapchain.present
        timer.await
    }
}