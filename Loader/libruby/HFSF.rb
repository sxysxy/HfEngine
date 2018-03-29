#encoding : utf-8
=begin
	HFSF.rb
		HfEngine Shaders Framework
    by sxysxy
=end

module HFSF

class Generator
	attr_accessor :list
	
	attr_accessor :name		    #optional
	attr_reader :native_objcet  #optional 
	def initialize
		@list = []
	end
	
	def method_missing(method_name, *arg, &block)
		g = method_name.to_s + "Generator"
		if eval("defined? #{g}")
			generator = eval("#{g}.new")
			generator.set_arg(*arg)
			generator.instance_eval(&block) if block
			@list.push generator
		else
			if @native_objcet.respond_to? method_name
				@native_objcet.__send__ method_name, *arg, &block
			else
				super
			end
		end
	end
end

class SamplerGenerator < Generator
	def set_arg(name)
		@name = name
		
		@native_objcet = DX::Sampler.new
		@native_objcet.use_default
	end
	
end

class ConstantBufferGenerator < Generator
	def set_arg(name)
		@name = name
	end
	
	def set_size(s)
	
	end
end

class ResourceGenerator < Generator
	def set_arg(name)
		@name = name
	end
end

class InputLayoutGenerator < Generator
	def set_arg(name)
		@name = name
	end
end

class SectionGenerator < Generator
	def set_arg(name)
		@name = name
	end
	
	def set_vshader(vs)
	
	end
end

class ProgramGenerator < Generator
	attr_reader :code
	def set_arg(name)
		@name = name
		@code = ""
	end
	
	define_method(:Code) {|c|
		@code = c
	}
end

class Compiler < Generator
	
	def self.compile_code(code)
		self.new.instance_eval(code)
	end
	def self.compile_file(filename)
		compile_code(File.read(filename))
	end
end

end #end of namespace HFSF


