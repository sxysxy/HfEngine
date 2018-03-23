=begin
module DX #namespace DX

class Shader
    def self.load_hlsl(device, filename, klass)
        s = klass.new
        s.create_from_hlsl(device, filename)
        return s
    end
    def self.load_string(device, str, klass)
        s = klass.new
        s.create_from_string(device, str)
        return s
    end
    def self.load_binfile(device, filename, klass)
        s = klass.new
        s.create_from_binfile(device, filename)
        return s
    end
end

[VertexShader, PixelShader].each {|klass|
	klass.instance_exec {
		def self.load_hlsl(device, filename)
			Shader.load_hlsl(device, filename, self)
		end
		def self.load_string(device, str)
			Shader.load_string(device, str, self)
		end
	}
}

end
=end