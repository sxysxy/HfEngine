Program("emm") {
	Code(%{
		//VS
		struct VSOut {
			float3 pos : POSITION;
			float4 color : COLOR;
		};
		VSOut VS(float3 pos : POSITION, float4 color : COLOR) {
			VSOut opt;
			opt.pos = pos;
			opt.color = color;
			return opt;
		}
		//GS
		struct GSOut {
			float4 pos : SV_POSITION;
			float4 color : COLOR;
		};
		cbuffer GSParam : register(b0) {
			float length; //length of each edge
			float em, emm, emmm; //useless, just fill to 16 bytes
		};
		[maxvertexcount(3)]
		void GS(point VSOut pts[1],
			inout TriangleStream<GSOut> opt) {  //output a triangle from a point
			GSOut top, left, right;
			//top
			top.pos.x = pts[0].pos.x;
			top.pos.y = pts[0].pos.y + length * 2.0 / 3.0 * sqrt(3.0) / 2.0;
			top.pos.z = pts[0].pos.z; top.pos.w = 1.0;
			top.color = pts[0].color;
			//left
			left.pos.x = pts[0].pos.x - length / 2.0;
			left.pos.y = pts[0].pos.y - length / 3.0 * sqrt(3.0) / 2.0;
			left.pos.z = pts[0].pos.z; left.pos.w = 1.0;
			left.color = pts[0].color;
			//right
			right.pos.x = pts[0].pos.x + length / 2.0;
			right.pos.y = pts[0].pos.y - length / 3.0 * sqrt(3.0) / 2.0;
			right.pos.z = pts[0].pos.z; right.pos.w = 1.0;
			right.color = pts[0].color;
			//in anti-clockwise
			[unroll] 
			opt.RestartStrip();
			opt.Append(left);
			opt.Append(right);
			opt.Append(top);	
		}
		float4 PS(GSOut emm) : SV_TARGET {
			return emm.color;
		}
	})
	InputLayout {
		Format "POSITION", DX::R32G32B32_FLOAT
		Format "COLOR", DX::R32G32B32A32_FLOAT
	}
	Resource {
		ConstantBuffer("GSParam") {
			set_size 16
			set_init_data [1.2, 0.0, 0.0, 0.0].pack("f*")
		}
	}
	Section("set") {
		set_vshader("VS")
		set_gshader("GS")
		set_pshader("PS")
		set_gs_cbuffer(0, "GSParam")
	}
}