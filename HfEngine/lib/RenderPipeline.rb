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

end
