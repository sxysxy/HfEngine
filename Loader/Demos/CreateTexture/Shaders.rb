Program("emm") {
	Code(%{
		struct VSOut {
			float4 pos : SV_POSITION;
			float2 tex : TEXCOORD;
		};
		VSOut VS(float2 pos : POSITION, float2 tex : TEXCOORD) {
			VSOut o;
			o.pos.xy = pos;
			o.pos.z = 0.0f, o.pos.w = 1.0f;
			o.tex = tex;
			return o;
		}		
		SamplerState ss : register(s0);
		Texture2D t2d : register(t0);
		float4 PS(VSOut o) : SV_TARGET {
			return t2d.Sample(ss, o.tex);
		}
	})
	InputLayout {
		Format "POSITION", R32G32_FLOAT
		Format "TEXCOORD", R32G32_FLOAT
	}
	Resource {
		Sampler("sampler") {
			use_default
		}
	}
	Section("set") {
		set_ps_sampler(0, "sampler")
		set_vshader("VS")
		set_pshader("PS")
	}
}