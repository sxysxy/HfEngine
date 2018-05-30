# HFSF DSL Reference

## Program :

  Program is a fragment which contains the primitive HLSL Codes, Resources, InputLayout, and Sections. To Create a
Program fragment, use Program(name) { ... }, a typical program may look like this:
```ruby
Program("Draw") {
	Code(%{
		//Primitive HLSL Codes.
		struct VSInput {
			//...
		};
		//...
	})
	InputLayout {
		#Describe the input layout, which indicates the input format of the vertexes. 
	}
	Resource {
		#Describe the resources you need(SamplerState, BlendState, ConstantBuffer, etc)
	}
	Section("set") {
		#Like Pass in Effects, you can set rendering state in Section fragment
	}
}
```

## InputLayout :

  InputLayout should be placed in a Program, In a InputLayout fragment, you can use keyword 'Format' to describe semantics. 
'Format' takes 2 arguments: semantic and data format. A typical InputLayout may look like this:
```ruby
	#...In a Program
	InputLayout {
		Format "POSITION", DX::R32G32_FLOAT
		Format "TEXCOORD", DX::R32G32_FLOAT
	}
	#...other parts
```     

## Resource :

  You can describe the resources you need in a Resource fragment. In Resource fragment, there are these keywords:
  
	Sampler 
	
	Blender
	
	Rasterizer
	
	DepthStencilState
	
	ConstantBuffer
	
  Each keyword requires a name as argument to identify itself, and a code block(ruby code block) to describe the resources 
In Sampler/Blender/Rasterizer/DepthStencilState, it will create a relative object in HfEngine(for example, Sampler will create a DX::Sampler) 
and call use_default method in HfEngine before execute your code block, so it's OK to give an empty descripting code block. 
But in ConstantBuffer, you should use set_size to set the size of the buffer, or you will get an Error. 
A typical Resources fragment may look like this:
```ruby
	#...In a Program
	Resource {
		Sampler("sampler") {
			#this code block will be instance_execed by a DX::Sampler
			#All methods in Class DX::Sampler can be used here
			set_filter DX::FILTER_MIN_MAG_MIP_LINEAR, 0
			set_uvwaddress DX::ADDRESS_WRAP, DX::ADDRESS_MIRROR, DX::ADDRESS_BORDER, HFColorRGBA(1.0, 1.0, 0.0, 1.0)
		}
		Blender{"alpha_blender"} {
			#this code block will be instance_execed by a DX::Blender
			enable true
			set_color_blend DX::BLEND_SRC_ALPHA, DX::BLEND_INV_SRC_ALPHA, DX::BLEND_OP_ADD 
			set_alpha_blend DX::BLEND_SRC_ALPHA, DX::BLEND_INV_SRC_ALPHA, DX::BLEND_OP_ADD
		}
		Rasterizer("rasterizer") {
			#this code block will be instance_execed by a DX::Rasterizer
			set_cull_mode DX::CULL_NONE
		}
		DepthStencilState("dss") {
			#this code block will be instance_execed by a DX::DepthStencilState
			set_depth_func DX::COMPARISON_LESS_EQUAL
		}
		ConstantBuffer("cb00") {
			#In ConstantBuffer, you have these two operations:
			set_size 16    #This is compulsory, if you forget to set it, HFSF will give an Error Message
			set_init_data [1.0, 0.0, 0.0, 0.0].pack("f*") #able to set initial data, this is optional
		}
		ConstantBuffer("cb01") {
			set_size 64
		}
	}
	#...other parts
```

## Section :
  Describe rendering state in Section fragment. You can use these keywords : 
```
  set_vshader(name), set_pshader(name), set_gshader(name)
  
  set_vs_sampler(slot, name), set_ps_sampler(slot, name), set_gs_sampler(slot, name),
  
  set_vs_cbuffer(slot, name), set_ps_cbuffer(slot, name), set_gs_cbuffer(slot, name),
  
  set_blender(name), set_rasterizer(name), set_depth_stencil_state(name)
```
  Shader's name is the function name in HLSL code, resources' name is the name you gave in Resource fragment, 
It's OK to pass nil, then it will clear its corresponding state.(for example. set_gshader(nil) will disable 
geometry shader stage, set_ps_cbuffer(0, nil) will set ConstantBuffer slot 0 to null) 
  slot is the slot number which you want to bind.  
  
	
	

