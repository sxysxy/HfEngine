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
		HFSF.loadsf(device, compiled)
			You should pass a DX::D3DDevice and a HFSF::Compiled
			This method returns an Array which contains your Programs(in HFSF) in order.
			example: 
				shaders = HFSF.loadsf(device, compiled)
				draw = shaders[0]
		HFSF.loadsf_code(device, &block)
		HFSF.loadsf_file(device, filename) 
			Load a shaders-framework directly from codes or a file, not through a HFSF::Compiled
		
	Examples : see Loader/libruby/Graphics2D/Renderer.rb 
			   and Loader/Demos/*
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
				
				begin
					generator.instance_eval(&block) if block
				rescue NameError => e
					msgbox e.message
				end
				
				@list.push generator
			else 
				
				super
			end
		end
		rescue Exception => e
			raise Exception, "HFSF Compile Error in #{method_name.to_s} #{arg.to_s}\n" + e.message
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
	def set_vshader(s)
		@vs = s
	end
	def set_vs_cbuffer(slot, b)
		@sets.push CBUFFER_INFO.new(:vs, b, slot)
	end
	def set_vs_sampler(slot, s)
		@sets.push SAMPLER_INFO.new(:vs, s, slot)
	end
	
	#PS
	def set_pshader(s)
		@ps = s
	end
	def set_ps_cbuffer(slot, b)
		@sets.push CBUFFER_INFO.new(:ps, b, slot)
	end
	def set_ps_sampler(slot, s)
		@sets.push SAMPLER_INFO.new(:ps, s, slot)
	end
	
	#GS
	def set_gshader(s)
		@gs = s
	end
	def set_gs_cbuffer(slot, b)
		@sets.push CBUFFER_INFO.new(:gs, b, slot)
	end
	def set_gs_sampler(slot, s)
		@sets.push SAMPLER_INFO.new(:gs, s, slot)
	end
	
	#OM
	def set_blender(b)
		@sets.push BLENDER_INFO.new(b)
	end
	
	#RS (Rasterizer)
	def set_rasterizer(r)
		@sets.push RASTERIZER_INFO.new(r)
	end
	
	#DepthStencilState
	def set_depth_stencil_state(dss)
		@sets.push DSS_INFO.new(dss)
	end
	
	def compile(context)
		pg = context[1] #should be a ProgramGenerator
		if !pg.is_a?(ProgramGenerator)
			raise GeneratingLogicError, "a big bug, please report..."
		end
		pg.compile_code(@vs, DX::VertexShader) if @vs != ""
		pg.compile_code(@ps, DX::PixelShader) if @ps != ""
		pg.compile_code(@gs, DX::GeometryShader) if @gs != ""
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
				x[:compiled][emm[0].to_sym] = DX::Shader.load_string(cp.device, @code, emm[1], emm[0]).byte_code
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
	
	attr_accessor :device
	def self.parse_code(code, device = nil)
		x = self.new 
		x.device = device ? device : DX::D3DDevice.new
		
		if code.is_a?(String)
			x.instance_eval(code)
		elsif code.is_a?(Proc)
			x.instance_exec &code
		end
		x.device.release if !device
		return x
	end
	
	def self.compile_code(code = nil, &block)
		d = DX::D3DDevice.new
		data = parse_code(code || block, d)	
		r = nil
		begin
			r = Compiled.new(data.compile([]), "#{VERSION} #{Time.now}")
		rescue Exception => e
			raise Exception, e.message
		ensure
			d.release
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
		[@blender, @sampler, @cbuffer, @rasterizer, @depth_stencil_state].each {|e|
			e.each {|name, s| s.release}
		}
	end
end

class SFInputLayout < SFData
	attr_accessor :idents, :formats
	
	def apply(rp)
		rp.set_input_layout(@idents, @formats)
	end
end

class SFSection < SFData
	attr_accessor :eval_code
	
	def initialize
		@eval_code = ""
	end
	
	def apply(rp)
		rp.instance_eval(@eval_code)
	end
end

#-------------

def self.load_section(device, program, sdata)
	section = SFSection.new
	
	#set vs
	#set_vshader(nil) will disable the vertex_shader
	if sdata[:vshader] && sdata[:vshader] != ""
		section.eval_code += "set_vshader(ObjectSpace._id2ref(#{program.shaders[sdata[:vshader].to_sym].object_id})) \n"
	elsif !sdata[:vshader]
		section.eval_code += "set_vshader(nil)\n"
	end
	
	#set gs
	if sdata[:gshader] && sdata[:gshader] != ""
		section.eval_code += "set_gshader(ObjectSpace._id2ref(#{program.shaders[sdata[:gshader].to_sym].object_id})) \n"
	elsif !sdata[:gshader]
		section.eval_code += "set_gshader(nil)\n"
	end
	
	#set ps
	if sdata[:pshader] && sdata[:pshader] != ""
		section.eval_code += "set_pshader(ObjectSpace._id2ref(#{program.shaders[sdata[:pshader].to_sym].object_id})) \n"
	elsif !sdata[:pshader]
		section.eval_code += "set_pshader(nil)\n"
	end
	
	#set other resources
	sdata[:sets].each {|set|
		case set
		when SectionGenerator::CBUFFER_INFO 
			if set.cbuffer.nil? #set to nullptr(example : set_ps_cbuffer(nil, 0), this will clear the cbuffer on slot 0
				section.eval_code += "set_#{set.stage.to_s}_cbuffer(#{set.slot}, nil)\n"
			else
				cb = program.resource.cbuffer[set.cbuffer.to_sym]
				raise GeneratingLogicError, "#{set.cbuffer} is not a DX::ConstantBuffer" if !cb.is_a?(DX::ConstantBuffer)
				section.eval_code += "set_#{set.stage.to_s}_cbuffer(#{set.slot}, ObjectSpace._id2ref(#{cb.object_id}))\n"
			end
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
		end
	}
	
	return section
end

def self.load_resource(device, program, rdata)
	resource = SFResource.new
	rdata.each {|name, element|
		#p = nil
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
	}
	return resource
end

def self.load_input_layout(device, program, iadata)
	x = SFInputLayout.new
	x.idents = iadata[:idents]
	x.formats = iadata[:formats]
	return x
end

def self.load_program(device, p)
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
		byte_code = a.pack("C*") 
		program.shaders[info[0].to_sym] = info[1].load_binary(device, byte_code, a.size)
	}
	#other
	
	#load resource
	resdata = p.find {|k, v| v[0] == HFSF::ResourceGenerator}
	if resdata
		program.resource = self.load_resource(device, program, resdata[1][1]) 
	else
		program.resource = SFResource.new
	end
	
	#after loading resources, load others
	program.section = {}
	p.each {|name, element|
		p = nil
		if element[0] == HFSF::InputLayoutGenerator
			p = self.load_input_layout(device, program, element[1])
			program.input_layout = p
=begin
		elsif element[0] == HFSF::ResourceGenerator
			p = self.load_resource(device, program, element[1])
			program.resource[name.to_sym] = p
=end
		elsif element[0] == HFSF::SectionGenerator
			p = self.load_section(device, program, element[1])
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

def self.loadsf(device, compd)
	#from top level
	a = []
	compd.row_hash.each {|name, element|
		p = self.load_program device, element[1]
		p.name = name.to_s
		a << p
	}
	return a
end

def self.loadsf_file(device, filename)
	compd = Compiler.compile_file(filename)
	self.loadsf(device, compd)
end

def self.loadsf_code(device, &block)
	self.loadsf(device, Compiler.compile_code(&block))
end

end #end of namespace HFSF


