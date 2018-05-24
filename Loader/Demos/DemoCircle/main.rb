require 'libcore'

SIZE = 500

c = HFSF::Compiler.compile_code {
	Program("e") {
		Code(%{
			cbuffer c : register(b0) {
				float4 color;
			};
		
			float4 VS(float3 pos : POSITION) : SV_POSITION {
				float4 f;
				f.xyz = pos;
				f.w = 1.0f;
				return f;
			}
			float4 PS(float4 pos : SV_POSITION) : SV_TARGET {
				float4 t = pos;
				float x = #{SIZE / 2};
				t.x -= x;
				t.y -= x;
				if(t.x * t.x + t.y * t.y <= 250*250)
					return color;
				else return float4(0.0f, 0.0f, 0.0f, 0.0f);
			}
		})
		InputLayout {
			Format "POSITION", DX::R32G32B32_FLOAT
		}
		Resource {
			ConstantBuffer("PSCB") {
				set_size 16
				set_init_data [1.0, 0.0, 1.0, 1.0].pack("f*")
			}
			Rasterizer("RS") {
				set_cull_mode DX::CULL_NONE
			}
		}
		Section("set") {
			set_vshader("VS")
			set_pshader("PS")
			set_ps_cbuffer(0, "PSCB")
			set_rasterizer("RS")
		}
	}
}

HFWindow.new("Circle", SIZE, SIZE) {
	show
	set_handler(:on_closed) {exit_mainloop}
	device = DX::D3DDevice.new
	rp = DX::RenderPipeline.new(device)
	sf = HFSF::loadsf(device, c)[0]
	sf.section[:set].apply(rp)
	sf.input_layout.apply(rp)
	swapchain = DX::SwapChain.new(device, self)
	rp.set_target(swapchain.rtt).set_viewport(HFRect(0, 0, width, height)).set_topology(DX::TOPOLOGY_TRIANGLESTRIP)
	vecs = [-1.0, 1.0, 0.0, 1.0, 1.0, 0.0, -1.0, -1.0, 0.0, 1.0, -1.0, 0.0].pack("f*")
	vb = DX::VertexBuffer.new(device, 12, 4, vecs)
	rp.set_vbuffer(vb)
	timer = FPSTimer.new(60)
	messageloop {
		rp.clear(HFColorRGBA(0.0, 0.0, 0.0, 0.0))
		rp.draw(0, 4)
		rp.immdiate_render
		swapchain.present
		timer.await
	}
}


