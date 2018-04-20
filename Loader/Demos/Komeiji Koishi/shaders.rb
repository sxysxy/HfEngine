Program("Draw"){
	Code(%{
	struct vs_output {
		float4 pos : SV_POSITION;
		float2 tex : TEXCOORD;
	};

	vs_output VS(float4 pos : POSITION, float2 tex : TEXCOORD) {
		vs_output opt;
		opt.pos = pos;
		opt.tex = tex;
		return opt;
	}

	cbuffer param : register(b0) {
		float rate;
		float em, emm, emmm; //useless
	};

	Texture2D koishi : register(t0);
	Texture2D yukari : register(t1);
	SamplerState color_sampler : register(s0);

	float4 PS(vs_output vpt) : SV_TARGET {
		return koishi.Sample(color_sampler, vpt.tex) * rate + yukari.Sample(color_sampler, vpt.tex) * (1 - rate);
	}
	})
	InputLayout {
		Format "POSITION", DX::R32G32_FLOAT
		Format "TEXCOORD", DX::R32G32_FLOAT
	}
	Resource {
		Sampler("linear_sampler") {
			use_default
		}
		Sampler("point_sampler") {
			use_default
			set_filter DX::FILTER_MIN_MAG_MIP_POINT, 0
		}
		Blender("blender") {
			use_default
			enable true
			set_mask COLOR_WRITE_ENABLE_ALL 
		}
		Rasterizer("rasterizer") {
			use_default
		}
		ConstantBuffer("param") {
			set_size 16
			set_init_data [0.5, 0.0, 0.0, 0.0].pack("f*")
		}
	}
	Section("set") {
		set_vshader("VS")
		set_pshader("PS")
		set_blender("blender")
		set_rasterizer("rasterizer")
		set_ps_cbuffer(0, "param")
		set_ps_sampler(0, "linear_sampler")
	}
}