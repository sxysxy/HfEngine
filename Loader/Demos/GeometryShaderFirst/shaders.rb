Program("emm") {
	Code(%{
		//VS
		struct VSOut {
			float3 pos : POSITION;
		};
		VSOut VS(float3 pos : POSITION) {
			VSOut opt;
			opt.pos = pos;
			return opt;
		}
		//GS
		struct GSOut {
			float4 pos : SV_POSITION;
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
			//left
			left.pos.x = pts[0].pos.x - length / 2.0;
			left.pos.y = pts[0].pos.y - length / 3.0 * sqrt(3.0) / 2.0;
			left.pos.z = pts[0].pos.z; left.pos.w = 1.0;
			//right
			right.pos.x = pts[0].pos.x + length / 2.0;
			right.pos.y = left.pos.y;
			right.pos.z = pts[0].pos.z; right.pos.w = 1.0;
			//in clockwise
			[unroll] 
			opt.RestartStrip();
			opt.Append(left);
			opt.Append(top);
			opt.Append(right);	
		}
		cbuffer PSParam : register(b1) {
			float4 color;
		};
		float4 PS(GSOut emm) : SV_TARGET {
			return color;
		}
	})
	InputLayout {
		Format "POSITION", DX::R32G32B32_FLOAT
	}
	Resource {
		ConstantBuffer("GSParam") {
			set_size 16
			set_init_data [1.2, 0.0, 0.0, 0.0].pack("f*")
		}
		ConstantBuffer("PSParam") {
			set_size 16
			set_init_data [0.0, 1.0, 0.0, 1.0].pack("f*")
		}
		Blender("blender") {
			set_mask COLOR_WRITE_ENABLE_ALL
		}
	}
	Section("set") {
		set_vshader("VS")
		set_gshader("GS")
		set_pshader("PS")
		set_gs_cbuffer(0, "GSParam")
		set_ps_cbuffer(1, "PSParam")
		set_blender("blender")
	}
}