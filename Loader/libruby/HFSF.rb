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
	attr_reader :list		    #parsed data
	attr_reader :compiled       #compiled data
	
	attr_accessor :name		    #optional, but most of them have.
	attr_reader :native_object  #optional 
	def initialize(name, nobj_klass = nil)
		@list = []
		@compiled = {}
		@name = name
		@native_object = nobj_klass.new if nobj_klass
	end
	      #valid_area
	      #for checking if valid to create something in some area.
	      #(eg. ConstantBuffer can not be created in a Section)

	
	def method_missing(method_name, *arg, &block)
		begin
		if @native_object.respond_to? method_name
			@native_object.__send__ method_name, *arg, &block
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
			puts "HFSF Compile Error in #{method_name.to_s} #{arg.to_s}" 
			raise e
		end
		
	end
	
	def compile(context)
		ncontext = context << self
		@list.each {|e|
			@compiled[e.name.to_sym] = [e.class, e.compile(ncontext)]
		}
		@compiled
	end
end

#Constructor for SamplerGenerator, BlenderGenerator, and RS...
# * means it is a terminal generator
class ResourcesBase 
	def self.new(native_obj_klass)
		return Class.new(Generator) {
			class_variable_set(:@@valid_area, :ResourceGenerator)
			define_method(:initialize) {|name|
				super(name, native_obj_klass)
				@native_object.use_default
			}
			define_method(:compile) {|context|
				{:row_data => @native_object.dump_description}
			}
		}
	end
end

#SamplerGenerator *
SamplerGenerator = ResourcesBase.new(DX::Sampler)

#BlenderGenerator *
BlenderGenerator = ResourcesBase.new(DX::Blender)

#Base abstract for BufferGenerators 
class BufferGeneratorBase < Generator
	attr_reader :size
	attr_reader :init_data
	def set_size(s)
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
	def compile(context)
		if !@size
			raise GeneratingLogicError, "#{self.class} #{self.name} : size is necessary for a buffer"
		end
		return {:size => @size, :init_data => @init_data ? @init_data.unpack("C*") : nil}
	end
end

#ConstantBufferGenerator *
class ConstantBufferGenerator < BufferGeneratorBase
	@@valid_area = :ResourceGenerator
	
	def set_size(s)
		if s % 16 != 0
			raise ArgumentError, "ConstantBuffer::set_size : size should be able to be dived by 16"
		end
		super(s)
	end
end

#ResourceGenerator
class ResourceGenerator < Generator
	@@valid_area = :ProgramGenerator
end

#InputLayoutGenerator *
class InputLayoutGenerator < Generator
	@@valid_area = :ProgramGenerator
	
	attr_reader :idents
	attr_reader :formats
	
	def initialize
		super("input_layout")
		@idents = []
		@formats = []
	end
	
	define_method(:Format) {|ident, format|
		@idents.push ident
		@formats.push format
	}
	
	def compile(context)
		return {:idents => @idents, :formats => @formats}
	end
end

#SectionGenerator *
class SectionGenerator < Generator
	@@valid_area = :ProgramGenerator
	
	attr_reader :vs, :ps
	CBUFFER_INFO = Struct.new(:stage, :cb, :slot)
	attr_reader :sets
	
	def initialize(name)
		super(name)
		@sets = []
	end
	def set_vshader(s)
		@vs = s
	end
	def set_pshader(s)
		@ps = s
	end
	def set_vs_cbuffer(b, slot = 0)
		@sets.push CBUFFER_INFO.new(:vs, b, slot)
	end
	def set_ps_cbuffer(b, slot = 0)
		@sets.push CBUFFER_INFO.new(:ps, b, slot)
	end
	
	def compile(context)
		pg = context[1] #should be a ProgramGenerator
		if !pg.is_a?(ProgramGenerator)
			raise GeneratingLogicError, "a big bug, please report..."
		end
		pg.compile_code(@vs, DX::VertexShader)
		pg.compile_code(@ps, DX::PixelShader)
		return {:vshader => @vs, :pshader => @ps, :sets => @sets}
	end
end

#ProgramGenerator
class ProgramGenerator < Generator
	@@valid_area = :Compiler
	
	attr_reader :code
	attr_reader :tobe_compiled
	
	def initialize(*arg)
		super(*arg)
		@tobe_compiled = []
	end
	
	define_method(:Code) {|c|
		@code = c
	}
			#entry function name, stage class (e.g. DX::VertexBuffer)
	def compile_code(entry, stage_klass)
		@tobe_compiled.push [entry, stage_klass]
	end
	
	def compile(context)
		x = super(context)
		x[:code] = @code
		x[:tobe_compiled] = @tobe_compiled
		return x
	end
end

#Compiler
class CompilerError < Exception
end
class Compiled
	attr_reader :row_data, :signature
	def initialize(r, s)
		@row_data = r
		@signature = s
	end
	
	def format_inspect_imp(x, retract)
		x.each {|key, value|
			if value.is_a?(Array) && value[1].is_a?(Hash)
				@text += "#{retract}#{value[0].to_s} #{key.to_s} {\n"
				format_inspect_imp(value[1], retract+"  ")
				@text += "#{retract}}\n"
			elsif 
				@text += "#{retract}#{key.to_s} => #{value.to_s} \n"
			end
		}
	end
	def format_inspect
		@text = ""
		format_inspect_imp(@row_data, "")
		"#{signature} \n#{@text}"
	end
	def save_file(filename)
		File.open(filename, "w") {|f|
			f.print format_inspect
		}
	end
	def save_marshal_file(filename)
		File.open(filename, "wb") {|f|
			f.write Marshal.dump(self)
		}
	end
	def self.load_marshal_file(filename)
		x = nil
		File.open(filename, "rb") {|f|
			x = Marshal.load(f.read) 
		}
		return x
	end
end

class Compiler < Generator
	VERSION = "HFSF Compiler v0.1"

	def initialize
		super(VERSION)
	end
	
	def self.parse_code(code)
		x = self.new
		begin
			if code.is_a?(String)
				x.instance_eval(code)
			elsif code.is_a?(Proc)
				x.instance_exec &code
			end
		rescue Exception => e
			puts e.message
		end
		return x
	end
	
	def self.compile_code(code = nil, &block)
		data = parse_code(code || block)	
		begin
			compd = Compiled.new(data.compile([]), "#{VERSION} #{Time.now}")
		rescue Exception => e
			raise e.message
		end
	end
	def self.compile_file(filename)
		compile_code(File.read(filename))
	end
end

end #end of namespace HFSF


