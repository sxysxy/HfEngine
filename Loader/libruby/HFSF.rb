#encoding : utf-8
=begin
	HFSF.rb
		HfEngine Shaders Framework
    by sxysxy
=end

module HFSF

class GeneratingLogicError < Exception
end

class Generator
	
	attr_accessor :list
	
	attr_accessor :name		    #optional, but most of them have.
	attr_reader :native_objcet  #optional 
	def initialize(name, nobj_klass = nil)
		@list = []
		@name = name
		@native_objcet = nobj_klass.new if nobj_klass
	end
	
	def self.set_valid_area
		msgbox 233
	end
	
	      #valid_area
	      #for checking if valid to create something in some area.
	      #(eg. ConstantBuffer can not be created in a Section)

	
	def method_missing(method_name, *arg, &block)
		begin
		
		if @native_objcet.respond_to? method_name
			@native_objcet.__send__ method_name, *arg, &block
		else
			g = method_name.to_s + "Generator"
			if eval("defined?(#{g})")
				new_area = HFSF.const_get(g)
				if self.class != HFSF.const_get(new_area.class_variable_get(:@@valid_area))
					raise GeneratingLogicError, "You should not create a #{g} in #{self.class}"
				end
			
				generator = new_area.new(*arg)
				generator.instance_eval(&block) if block
				
				@list.push generator
			else
				super
			end
		end
		
		rescue Exception => e
			puts "HFSF Complie Error in #{method_name.to_s} #{arg.to_s}" 
			raise e
		end
		
	end
end


#SamplerGenerator
class SamplerGenerator < Generator
	@@valid_area = :ResourceGenerator
	
	def initialize(name)
		super(name, DX::Sampler)
		@native_objcet.use_default
	end
	
end

class BlenderGenerator < Generator
	@@valid_area = :ResourceGenerator
	
	def initialize(name)
		super(name, DX::Blender)
		@native_objcet.use_default
	end
end

#ConstantBufferGenerator
class ConstantBufferGenerator < Generator
	@@valid_area = :ResourceGenerator

	attr_reader :size
	attr_reader :init_data
	def set_size(s)
		if s % 16 != 0
			raise ArgumentError, "set_size : size should be able to be dived by 16"
		end
		@size = s
	end
	def set_init_data(d)
		if !@size.is_a?(Integer)
			raise GeneratingLogicError, "set_init_data : size is still unknown"
		end
		if d.is_a?(String) || d.is_a?(Integer)
			@init_data = d
		else
			raise ArgumentError, "set_init_data : data can be a string(packed from an array) or a Integer(a Pointer)"
		end
	end
	
end

#ResourceGenerator
class ResourceGenerator < Generator
	@@valid_area = :ProgramGenerator
end

#InputLayoutGenerator
class InputLayoutGenerator < Generator
	@@valid_area = :ProgramGenerator
end

#SectionGenerator
class SectionGenerator < Generator
	@@valid_area = :ProgramGenerator
	
	attr_reader :vs, :ps
	
	def set_vshader(s)
		@vs = s
	end
	
	def set_pshader(s)
		@ps = s
	end
	
	def set_vs_cbuffer(b, slot = 0)
		
	end
	
	def set_ps_cbuffer(b, slot = 0)
	
	end
end

#ProgramGenerator
class ProgramGenerator < Generator
	@@valid_area = :Compiler
	
	attr_reader :code
	
	define_method(:Code) {|c|
		@code = c
	}
end

#Compiler
class Compiler < Generator
	def initialize
		super("HFSF Compiler v0.1")
	end
	
	def self.compile_code(code)
		x = self.new
		begin
			x.instance_eval(code)
		rescue Exception => e
			puts e.message
			puts $@
		end
		return x
	end
	def self.compile_file(filename)
		compile_code(File.read(filename))
	end
end

end #end of namespace HFSF


