class ConfigHolder
    attr_reader :config
    def initialize
        @config = {}
    end
    def method_missing(name, *arg, &block)
        if @config[name.to_sym] && arg.size > 0
            raise ArgumentError, "#{name} attribute has been set before"
        end
        return @config[name.to_sym] if arg.size == 0
        @config[name.to_sym] = arg.size == 1 ? arg[0] : arg

    end
end

class ConfigLoader
    attr_reader :config
    def initialize
        @config = {}
    end
    define_method(:Config) do |name, &block|
        if @config[name.to_sym]
            raise ArgumentError, "Config Section #{name} has already been in existance"
        end
        x = ConfigHolder.new
        x.instance_exec(&block)
        @config[name.to_sym] = x
    end
    def [](name) 
        if @config[name.to_sym] 
            return @config[name.to_sym]
        else
            #raise ArgumentError, "#{name} config section not found"
			nil
        end
    end

    def self.load(filename)
        x = self.new
        x.instance_eval(File.read(filename))
        return x
    end
end
