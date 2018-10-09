Program("Draw"){
	Code(%{
	cbuffer wvpmatrix : register(b0) {
		float4x4 wvp;
	};

	struct vs_output{
		float4 pos : SV_POSITION;
		float4 color : COLOR;
	};

	vs_output VS(float4 pos : POSITION, float4 color : COLOR) {
		vs_output opt;
		opt.pos = mul(pos, wvp);
		opt.color = color;
		return opt;
	}

	float4 PS(vs_output v) : SV_TARGET {
		return v.color;
	}
	})
	InputLayout {
		Format "POSITION", DX::R32G32B32_FLOAT
		Format "COLOR", DX::R32G32B32A32_FLOAT
	}
	Resource {
		ConstantBuffer("wvpmatrix") {
			set_size 64
		}
		Rasterizer("rs") {
			use_default
		}
	}
	Section("set") {
		set_vshader("VS")
		set_pshader("PS")
		set_rasterizer("rs")
		set_vs_cbuffer(0, "wvpmatrix")
	}
}


