#HFSF DSL Reference

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
		//Describe the input layout, which indicates the input format of the vertexes. 
	}
	Resource {
		//Describe the resources you need(SamplerState, BlendState, ConstantBuffer, etc)
	}
	Section("set") {
		//Like Pass in Effects, you can set rendering state in Section fragment
	}
}
```

