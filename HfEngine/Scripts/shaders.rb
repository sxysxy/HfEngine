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
		Format "POSITION", 2
		Format "COLOR", 2
	}
	Resource {
		ConstantBuffer("wvpmatrix") {
			size 64
		}
	}
	Section("set") {
		vshader("VS")
		pshader("PS")
		
		vs_cbuffer(0, "wvpmatrix")
    }
}


