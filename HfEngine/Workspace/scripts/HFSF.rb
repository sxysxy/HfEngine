#encoding : utf-8
=begin
HFSF.rb
	HfEngine Shaders Framework
by sxysxy

  This is a shaders framework, like Effects, which helps you manage your shaders and relative resources more conviniently.
  It mainly contains two parts:
	1. HFSF DSL. A DSL descripting your shaders framework.
	2. HFSF Runtime. It allows you access the resources descripted in HSFS DSL and execute the codes in Section fragments.
  It can manage VertexShader, PixelShader, GeometryShader, SamplerState, BlendState, RasterizerState, DepthStencilState, 
	ConstantBuffer, InputLayout, you can just descript them in the DSL, and HSFS Runtime will create then when loading it.
  You can also set rendering state in DSL(in Section fragments), like Pass fragments in Effects.
  HFSF is fully compatible with ruby, so all ruby features can be used in it, for example, you can use some ruby's tricks to
    pass arguments to Section fragments.(This feature is not supported offically because it's too ruby, I want its binary 
	file(HFSF DSL can be compiled into universal binary file) can be used by C++). HFSF DSL has syntax and logic reviewing, 
	and can give you friendly error massages if your codes have something wrong.
  for more details about HFSF DSL, see the HFSF DSL Document(Document/HFSF_DSL.md)
  The whole HFSF system is totally written in Ruby(700+ lines), cool!
	
Interfaces:
	Compile your HFSF Code:
		compiled = HFSF.compile_code(code = nil, &block)
			You can pass a String or a block representing your code, Recommand passing a block.
		compiled = HFSF.compile_file(filename)
			Complie a file
	The two methods above returns a HFSF::Compiled, it contains the compiled data. 
	
	HFSF::Compiled: 
		compiled.format_inspect
			Inspect the compile data, this method will return a String, you can use 'print' to print it.
		compiled.save_sfm
			Save the compiled data into a binary file.(using ruby Marshal)
		compiled.load_sfm
			Load a binary file which was saved by using save_sfm
	
	Load a shaders-framework: 
		HFSF.loadsf(compiled)
			This method returns an Array which contains your Programs(in HFSF) in order.
			example: 
				shaders = HFSF.loadsf(device, compiled)
				draw = shaders[0]
		HFSF.loadsf_code(device, &block)
		HFSF.loadsf_file(device, filename) 
			Load a shaders-framework directly from codes or a file, not through a HFSF::Compiled
	
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
			if HFSF.const_defined?(g)
				new_area = HFSF.const_get(g)
				if self.class != HFSF.const_get(new_area.class_variable_get(:@@valid_area))
					raise GeneratingLogicError, "You should not create a #{g} in #{self.class}"
				end
				generator = new_area.new(*arg)
				
				generator.instance_exec(&block) if block
				@list.push generator
			else 
				super
			end
		end
		rescue Exception => e
			msg = "\n\tHFSF Compile Error in #{method_name.to_s} #{arg.to_s}" + ", raised by #{e.backtrace[0]}"
			msg += " #{e.message}" if e.message && e.message.length > 0 
			#if self.class != ProgramGenerator
				raise Exception, msg
			#else  
				#puts msg
				#puts "\n\t:backtrace array: #{e.backtrace}\n"
				#raise Exception, "HFSF Compile Error"
			#end
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
			class_variable_set(:@@native_obj_klass, native_obj_klass)
			def initialize(name)
				super(name, @@native_obj_klass)
				@native_object.use_default
			end
			def compile(context)
				{:raw_data => @native_object.__dump__}
			end
		}
	end
end

SamplerGenerator = ResourcesBase.new(HEG::Sampler)

=begin
#BlenderGenerator *
BlenderGenerator = ResourcesBase.new(DX::Blender)
class BlenderGenerator < Generator
	alias :old_compile :compile
	def compile(context)
		d = old_compile(context)
		c = @native_object.blend_factor
		d[:blend_factor] = c ? ([c.r, c.g, c.b, c.a]) : [0.0, 0.0, 0.0, 0.0]
		return d
	end
end

#RasterizerGenerator *
RasterizerGenerator = ResourcesBase.new(DX::Rasterizer)

#DepthStencilState
DepthStencilStateGenerator = ResourcesBase.new(DX::DepthStencilState)
=end

#Base abstract for BufferGenerators 
class BufferGeneratorBase < Generator
	attr_reader :size
	attr_reader :init_data
	def size(s)
		@size = s
	end
	def init_data(d)
		if !@size.is_a?(Integer)
			raise GeneratingLogicError, "init_data : size is still unknown"
		end
		if d.is_a?(String) || d.is_a?(Integer)
			@init_data = d
		else
			raise ArgumentError, "init_data : data can be a string(packed from an array) or a Integer(a Pointer)"
		end
	end
	def compile(context)
		if !@size
			raise GeneratingLogicError, "#{self.class} #{self.name} : size is necessary for a buffer"
		end
		return {:size => @size, :init_data => @init_data ? @init_data.unpack("C*") : nil}
	end
end

#Remove buffer support
#ConstantBufferGenerator *
=begin
class ConstantBufferGenerator < BufferGeneratorBase
	@@valid_area = :ResourceGenerator
	
	def size(s)
		if s % 16 != 0
			raise ArgumentError, "ConstantBuffer::size : size should be able to be dived by 16"
		end
		super(s)
	end
end
=end

#ResourceGenerator
class ResourceGenerator < Generator
	@@valid_area = :ProgramGenerator
	
	def initialize()
		super("")
	end
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
	alias :format :Format
	
	def compile(context)
		return {:idents => @idents, :formats => @formats}
	end
end

#SectionGenerator *
class SectionGenerator < Generator
	@@valid_area = :ProgramGenerator
	
	attr_reader :vs, :ps, :gs
	CBUFFER_INFO = Struct.new(:stage, :cbuffer, :slot)
	SAMPLER_INFO = Struct.new(:stage, :sampler, :slot)
	
	BLENDER_INFO = Struct.new(:blender)
	RASTERIZER_INFO = Struct.new(:rasterizer)
	DSS_INFO = Struct.new(:depth_stencil_state)
	attr_reader :sets
	
	def initialize(name)
		super(name)
		@sets = []
					
		@vs = "" #remain state
		@ps = ""
		@gs = ""
	end
	
	#VS
	def vshader(s)
		@vs = s
	end
	def vs_cbuffer(slot, b)
		@sets.push CBUFFER_INFO.new(:vs, b, slot)
	end
	def vs_sampler(slot, s)
		@sets.push SAMPLER_INFO.new(:vs, s, slot)
	end
	
	#PS
	def pshader(s)
		@ps = s
	end
	def ps_cbuffer(slot, b)
		@sets.push CBUFFER_INFO.new(:ps, b, slot)
	end
	def ps_sampler(slot, s)
		@sets.push SAMPLER_INFO.new(:ps, s, slot)
	end
	
	#GS
	def gshader(s)
		@gs = s
	end
	def gs_cbuffer(slot, b)
		@sets.push CBUFFER_INFO.new(:gs, b, slot)
	end
	def gs_sampler(slot, s)
		@sets.push SAMPLER_INFO.new(:gs, s, slot)
	end
=begin
	#OM
	def blender(b)
		@sets.push BLENDER_INFO.new(b)
	end
	
	#RS (Rasterizer)
	def rasterizer(r)
		@sets.push RASTERIZER_INFO.new(r)
	end
	
	#DepthStencilState
	def depth_stencil_state(dss)
		@sets.push DSS_INFO.new(dss)
	end
=end
	
	def compile(context)
		pg = context[1] #should be a ProgramGenerator
		if !pg.is_a?(ProgramGenerator)
			raise GeneratingLogicError, "a big bug, please report..."
		end
		pg.compile_code(@vs, HEG::VertexShader) if @vs != ""
		pg.compile_code(@ps, HEG::PixelShader) if @ps != ""
		pg.compile_code(@gs, HEG::GeometryShader) if @gs != ""
		return {:vshader => @vs, :pshader => @ps, :gshader => @gs, :sets => @sets}
	end
end

#ProgramGenerator
class ProgramGenerator < Generator
	@@valid_area = :Compiler
	
	attr_reader :code
	attr_reader :code_line_number
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
		if !@code
			raise GeneratingLogicError, "No HLSL Code, compile failed"
		end
		cp = context[0]
		if !cp.is_a?(Compiler)
			raise "Big bug, please report..."
		end
		
		x[:code] = @code
		x[:tobe_compiled] = @tobe_compiled
		
		x[:compiled] = {}
		@tobe_compiled.each do |emm|
			if !x[:compiled][emm[0].to_sym]  #Avoid re-compiling
				shader =  emm[1].new.from_string(@code, emm[0])
				x[:compiled][emm[0].to_sym] = shader.byte_code
				shader.release
				#x[:compiled][emm[0].to_sym] = emm[1].new.from_string(@code, emm[0])
			end
		end
		return x
	end
end

#Compiler
class CompilerError < Exception
end
class Compiled
	attr_reader :row_hash, :signature
	def initialize(r, s)
		@row_hash = r
		@signature = s
	end
	
	def format_inspect_imp(x, retract)
		x.each {|key, value|
			if value.is_a?(Array) && value[1].is_a?(Hash)
				@text += "#{retract}#{value[0].to_s} #{key.to_s} {\n"
				format_inspect_imp(value[1], retract+"  ")
				@text += "#{retract}}\n"
			elsif value.is_a?(Hash)
				@text += "#{key.to_s} => {\n"
				format_inspect_imp(value, retract+"  ")
				@text += "#{retract}}\n"
			else
				@text += "#{retract}#{key.to_s} => #{value.to_s} \n"
			end
		}
	end
	def format_inspect
		@text = ""
		format_inspect_imp(@row_hash, "")
		"#{signature} \n#{@text}"
	end
	alias :to_s :format_inspect
	alias :Inspect :format_inspect

	def save_file(filename)
		File.open(filename, "w") {|f|
			f.print format_inspect
		}
	end

	def save_sfm(filename)
		if File.extname(filename) == "" 
			filename += ".sfm"
		end
		File.open(filename, "wb") {|f|
			f.write Marshal.dump(self)
		}
	end

	def save_sfo(filename)
		raise "save_sfo, not imp"
	end

	def self.load_sfm(filename)
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
		
		if code.is_a?(String)
			x.instance_eval(code)
		elsif code.is_a?(Proc)
			x.instance_exec &code
		end
		
		return x
	end
	
	def self.compile_code(code = nil, &block)
		data = parse_code(code || block)	
		r = nil
		begin
			r = Compiled.new(data.compile([]), "#{VERSION} #{Time.now}")
		rescue Exception => e
			raise Exception, e.message
		end
		return r
	end
	def self.compile_file(filename)
		compile_code(File.read(filename))
	end
end

class SFData
	attr_accessor :name
end

class SFProgram < SFData
	attr_accessor :shaders
	attr_accessor :resource
	attr_accessor :section
	attr_accessor :input_layout
	alias :layout :input_layout
	
=begin
	#copy. It does nnt copy constant buffer's data.
	def copy(device)
		p = SFProgram.new
		p.shaders = @shaders
		p.section = @section
		p.input_layout = @input_layout
		#copy resource
		p.resource = @resource.copy(device) if @resource
		return p
	end
=end
	def release
		@shaders.each{|name, shader| shader.release}
		@resource.release
	end
end

class SFResource < SFData
	attr_accessor :blender
	attr_accessor :sampler
	attr_accessor :cbuffer
	attr_accessor :rasterizer
	attr_accessor :depth_stencil_state
	def initialize
		@blender = {}
		@sampler = {}
		@cbuffer = {}
		@rasterizer = {}
		@depth_stencil_state = {}
	end
=begin
	#copy. It does nnt copy constant buffer's data.
	def copy(device)
		r = SFResource.new
		@blender.each { |name, obj|
			s = DX::Blender.new
			s.load_description(obj.dump_description.pack("C*"))
			s.set_blend_factor obj.blend_factor
			s.create_state(device)
			r.blender[name] = s
		}
		@sampler.each { |name, obj|
			s = DX::Sampler.new
			s.load_description(obj.dump_description.pack("C*"))
			s.create_state(device)
			r.sampler[name] = s
		}
		@cbuffer.each { |name, obj|
			s = DX::ConstantBuffer.new(device, obj.size, 0)
			r.cbuffer[name] = s
		}
		@rasterizer.each { |name, obj|
			s = DX::Rasterizer.new
			s.load_description(obj.dump_description.pack("C*"))
			s.create_state(device)
			r.rasterizer[name] = s
		}
		return r
	end
=end
	def release
		#[@blender, @sampler, @cbuffer, @rasterizer, @depth_stencil_state].each {|e|
		#	e.each {|name, s| s.release}
		#}
		@cbuffer.each {|name, s| s.release}
	end

	
end

class SFInputLayout < SFData
	attr_accessor :idents, :formats
	
	def apply(rc)
		rc.layout(@idents, @formats)
	end
end

class SFSection < SFData
	attr_accessor :eval_code
	
	def initialize
		@eval_code = ""
	end
	
	def apply(rc)
		rc.instance_eval(@eval_code)
	end
end

#-------------

def self.load_section(program, sdata)
	section = SFSection.new
	
	#set vs
	#set_vshader(nil) will disable the vertex_shader
	if sdata[:vshader] && sdata[:vshader] != ""
		section.eval_code += "shader(ptr2object(#{object2ptr(program.shaders[sdata[:vshader].to_sym])})) \n"
	elsif !sdata[:vshader]
		section.eval_code += "shader(nil)\n"
	end
	
	#set gs
	if sdata[:gshader] && sdata[:gshader] != ""
		section.eval_code += "shader(ptr2object(#{object2ptr(program.shaders[sdata[:gshader].to_sym])})) \n"
	elsif !sdata[:gshader]
		section.eval_code += "shader(nil)\n"
	end
	
	#set ps
	if sdata[:pshader] && sdata[:pshader] != ""
		section.eval_code += "shader(ptr2object(#{object2ptr(program.shaders[sdata[:pshader].to_sym])})) \n"
	elsif !sdata[:pshader]
		section.eval_code += "shader(nil)\n"
	end
	
	stage_table = {:vs => HEG::Shader::VERTEX, :ps => HEG::Shader::PIXEL, :gs => HEG::Shader::GEOMETRY}
	#set other resources
	sdata[:sets].each {|set|
		case set
		when SectionGenerator::CBUFFER_INFO 
			if set.cbuffer.nil? #set to nullptr(example : cbuffer(nil, 0), this will clear the cbuffer on slot 0
				section.eval_code += "cbuffer(#{stage_table[set.stage]}, #{set.slot}, nil)\n"
			else
				cb = program.resource.cbuffer[set.cbuffer.to_sym]
				section.eval_code += "cbuffer(#{stage_table[set.stage]}, #{set.slot}, ptr2object(#{object2ptr(cb)}))\n"
			end
		when SectionGenerator::SAMPLER_INFO
			if set.sampler.nil?
				section.eval_code += "sampler(#{stage_table[set.stage]}, #{set.slot}, nil)\n"
			else
				s = program.resource.sampler[set.sampler.to_sym]
				section.eval_code += "sampler(#{stage_table[set.stage]}, #{set.slot}, ptr2object(#{object2ptr(s)}))\n"
			end
=begin
		when SectionGenerator::BLENDER_INFO
			if set.blender.nil?
				section.eval_code += "set_blender(nil)\n"
			else
				b = program.resource.blender[set.blender.to_sym]
				raise GeneratingLogicError, "#{set.blender} is not a DX::Blender" if !b.is_a?(DX::Blender)
				section.eval_code += "set_blender(ObjectSpace._id2ref(#{b.object_id}))\n"
			end
		when SectionGenerator::SAMPLER_INFO
			if set.sampler.nil?
				section.eval_code += "set_#{set.stage.to_s}_sampler(#{set.slot}, nil)\n"
			else
				s = program.resource.sampler[set.sampler.to_sym]
				raise GeneratingLogicError, "#{set.sampler} is not a DX::Sampler" if !s.is_a?(DX::Sampler)
				section.eval_code += "set_#{set.stage.to_s}_sampler(#{set.slot}, ObjectSpace._id2ref(#{s.object_id}))\n"
			end
		when SectionGenerator::RASTERIZER_INFO
			if set.rasterizer.nil?
				section.eval_code += "set_rasterizer(nil)\n"
			else
				r = program.resource.rasterizer[set.rasterizer.to_sym]
				raise GeneratingLogicError, "#{set.rasterizer} is not a DX::Rasterizer" if !r.is_a?(DX::Rasterizer)
				section.eval_code += "set_rasterizer(ObjectSpace._id2ref(#{r.object_id}))\n"
			end
		when SectionGenerator::DSS_INFO
			if set.depth_stencil_state.nil?
				section.eval_code += "set_depth_stencil_state(nil)\n"
			else
				r = program.resource.depth_stencil_state[set.depth_stencil_state.to_sym]
				raise GeneratingLogicError, "#{set.depth_stencil_state} is not a DX::DepthStencilState" if !r.is_a?(DX::DepthStencilState)
				section.eval_code += "set_depth_stencil_state(ObjectSpace._id2ref(#{r.object_id}))\n"
			end
=end
		end

	}
	
	return section
end

def self.load_resource(program, rdata)
	resource = SFResource.new
	rdata.each {|name, element|
		#p = nil
=begin
		if element[0] == BlenderGenerator
			s = DX::Blender.new
			s.load_description element[1][:row_data].pack("C*")
			c = element[1][:blend_factor]
			s.set_blend_factor HFColorRGBA(c[0], c[1], c[2], c[3])
			s.create_state(device)
			resource.blender[name.to_sym] = s
		elsif element[0] == ConstantBufferGenerator
			cb = DX::ConstantBuffer.new(device, element[1][:size], 
						element[1][:init_data] ? element[1][:init_data].pack("C*") : 0)
			resource.cbuffer[name.to_sym] = cb
		elsif element[0] == SamplerGenerator
			s = DX::Sampler.new
			s.load_description element[1][:row_data].pack("C*")
			s.create_state(device)
			resource.sampler[name.to_sym] = s
		elsif element[0] == RasterizerGenerator
			r = DX::Rasterizer.new
			r.load_description element[1][:row_data].pack("C*")
			r.create_state(device)
			resource.rasterizer[name.to_sym] = r
		elsif element[0] == DepthStencilStateGenerator
			ds = DX::DepthStencilState.new
			ds.load_description element[1][:row_data].pack("C*")
			ds.create_state(device)
			resource.depth_stencil_state[name.to_sym] = ds
		end
=end
		if element[0] == SamplerGenerator
			s = HEG::Sampler.new 
			s.__load__(element[1][:raw_data])
			s.create_state
			resource.sampler[name.to_sym] = s
		end
	}
	return resource
end

def self.load_input_layout(program, iadata)
	x = SFInputLayout.new
	x.idents = iadata[:idents]
	x.formats = iadata[:formats]
	return x
end

def self.load_program(p)
	program = SFProgram.new
	
	#byte code
	program.shaders = {}
	p[:tobe_compiled].each {|info|
		#program.byte_code[info[0].to_sym] = p[:compiled][info[0].to_sym].pack("C*") 
				#info[0] is shader entry's name, info[1] is the class(such as VertexShader)
		a = p[:compiled][info[0].to_sym]
		if !a
			raise GeneratingLogicError, "Can not find shader : #{info[0].to_s}"
		end
		program.shaders[info[0].to_sym] = info[1].new.from_binary(a)
	}
	#other
	
	#load resource
	resdata = p.find {|k, v| v[0] == HFSF::ResourceGenerator}
	if resdata
		program.resource = self.load_resource(program, resdata[1][1]) 
	else
		program.resource = SFResource.new
	end
	
	#after loading resources, load others
	program.section = {}
	p.each {|name, element|
		p = nil
		if element[0] == HFSF::InputLayoutGenerator
			p = self.load_input_layout(program, element[1])
			program.input_layout = p
=begin
		elsif element[0] == HFSF::ResourceGenerator
			p = self.load_resource(device, program, element[1])
			program.resource[name.to_sym] = p
=end
		elsif element[0] == HFSF::SectionGenerator
			p = self.load_section(program, element[1])
			program.section[name.to_sym] = p
		else
			#msgbox element[0]
			#raise GeneratingLogicError, "Unknown : #{element[0]}"
		end
		p.name = name.to_s if p
	}
	return program
end


#Public Interfaces:
def self.compile_code(code = nil, &block) 
	return Compiler.compile_code(code, &block)
end

def self.compile_file(filename)
	return Compiler.compile_file(filename)
end

def self.loadsf(compd)
	compd.row_hash.each {|name, element|
		p = self.load_program element[1]
		p.name = name.to_s
		return p
	}
	return nil
end

def self.loadsf_multi(compd)
	a = []
	compd.row_hash.each {|name, element|
		p = self.load_program element[1]
		p.name = name.to_s
		a << p
	}
	return a
end

def self.loadsf_file(filename)
	compd = Compiler.compile_file(filename)
	self.loadsf(compd)
end

def self.loadsf_code(&block)
	compd = Compiler.compile_code(&block)
	self.loadsf(compd)
end

def self.loadsf_multi_file(filename)
	compd = Compiler.compile_file(filename)
	self.loadsf_multi(compd)
end

def self.loadsf_multi_code(&block)
	compd = Compiler.compile_code(&block)
	self.loadsf_multi(compd)
end

end #end of namespace HFSF


