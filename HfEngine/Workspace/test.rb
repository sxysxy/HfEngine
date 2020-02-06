#encoding: utf-8
HEG::GDevice.create   #创建设备单例
VertexShaderCode = "
float4 main(float2 pos : POS) : SV_POSITION {
    return float4(pos.x, pos.y, 0.5, 1.0);    
}
"
PixelShaderCode = "
float4 main(float4 pos : SV_POSITION) : SV_TARGET {
    return float4(0.8, 0.5, 0.2, 1.0);
}
"
HEG::Window.new("HfEngine Demo", 800, 600).instance_exec {
    show()
    handle(:closed) { exit_process 0 }
    context = HEG::RenderContext.new
    context.shader(HEG::VertexShader.new.from_string(VertexShaderCode))
    context.shader(HEG::PixelShader.new.from_string(PixelShaderCode))
    context.layout(["POS"], [HEG::FORMAT_FLOAT2])
    context.vbuffer(HEG::VertexBuffer.new(3, 4 * 2, pack_floats([0.0, 1.0, -1.0, -1.0, 1.0, -1.0])))
    context.viewport(0, 0, width(), height())
    context.target(canvas())
    #timer = HEG::FPSTimer.new(HEG::GDevice.instance.monitor_info[:refresh_rate])
    mainloop {
        context.clear
        context.draw(HEG::TOPOLOGY_TRIANGLES, 0, 3)
        context.render

        #timer.wait
        swap_buffers()
    }
}