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
		Sampler("sampler") {}
		Blender{"blender"} {}
		Rasterizer("rasterizer") {}
		DepthStencilState("dss") {}
		ConstantBuffer("cb00") {
			set_size 16
		}
		ConstantBuffer("cb01") {
			set_size 64
		}
	}
	#...other parts
```

(to be continued)
	
	

