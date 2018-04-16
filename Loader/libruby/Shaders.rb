module DX #namespace DX

class Shader
    def self.load_hlsl(device, filename, klass, entry = "main")
        s = klass.new
        s.create_from_hlsl(device, filename, entry)
        return s
    end
    def self.load_string(device, str, klass, entry = "main")
        s = klass.new
        s.create_from_string(device, str, entry)
        return s
    end
    def self.load_binary(device, x, size, klass)
		s = klass.new
		s.create_from_binary(device, x, size)
        return s
    end
end

[VertexShader, PixelShader].each {|klass|
	klass.instance_exec {
		def self.load_hlsl(device, filename, entry = "main")
			Shader.load_hlsl(device, filename, self, entry)
		end
		def self.load_string(device, str, entry = "main")
			Shader.load_string(device, str, self, entry)
		end
		def self.load_binary(device, x, size)
			Shader.load_binary(device, x, size, self)
		end
	}
}

end
