#encoding :utf-8

module HEG::FFI
    module CStruct 
        def self.new(&block)
            cla = Class.new(::Object) {
                class_variable_set(:@@members_sym, [])
                class_variable_set(:@@members_type, {})
                
                @@members_type = {}
                class << self
                    members_sym = class_variable_get(:@@members_sym)
                    members_type = class_variable_get(:@@members_type)
                    define_method(:int32) do |*member|
                        member.each do |m| 
                            s = m.to_sym
                            members_sym << s 
                            members_type[s] = 'L'
                        end
                    end 

                    define_method(:int64) do |*member|
                        member.each do |m| 
                            s = m.to_sym
                            members_sym << s 
                            members_type[s] = 'Q'
                        end
                    end 
                    
                    alias :int :int32
                    alias :vptr :int64 
                   
                    define_method(:float) do |*member|
                        member.each do |m| 
                            s = m.to_sym
                            members_sym << s 
                            members_type[s] = 'e'
                        end
                    end 
                    
                    define_method(:double) do |*member|
                        member.each do |m| 
                            s = m.to_sym
                            members_sym << s 
                            members_type[s] = 'E'
                        end
                    end 
                    
                    define_method(:cstr) do |*member|
                        member.each do |m| 
                            s = m.to_sym
                            members_sym << s 
                            members_type[s] = 'A'
                        end
                    end 
                end

                def initialize
                    @members_value = {}
                    @packed = nil
                end
                def [](name)
                    return @members_value[name.to_sym]
                end
                def []=(name, v)
                    @members_value[name.to_sym] = v
                end
                #def method_missing(name, *arg, &b)
                #    return @members_value[name] if @members_value[name]
                #    super
                #end
                def pack
                    p = []
                    d = []
                    @@members_sym.each {|s| 
                        d << @members_value[s]
                        p << @@members_type[s]
                    }
                    #msgbox "x", "#{d} #{p.join} \n #{d.pack(p.join).bytes.size} \n #{d.pack(p.join).bytes}"
                    @packed = d.pack(p.join) #Avoid GC free it
                    zeors = []
                    s = @packed.size 
                    while s % 8 != 0 #Align to 8 bytes
                        zeors << "\0"
                        s += 1
                    end
                    @packed += zeors.join 
                    @packed
                end

                def unpack(data)
                    p = []
                    @@members_sym.each {|s| 
                        p << @@members_type[s]
                    }
                    return data.unpack(p.join)
                end

                def inspect
                    ins = []
                    @@members_sym.each {|s| 
                        ins << "#{s.to_s} = #{@members_value[s]}"
                    }
                    ins.join("\n")
                end
            }
            cla.instance_exec &block
            return cla
        end
    end
    
end