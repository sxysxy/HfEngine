#encoding :utf-8
module HEG::FFI
    class Module
        
        #[DOCUMENT]
        #attribute: dll
        #note: DLL object of the module
        attr_reader :dll

        #[DOUMENT]
        #method: initialize(dllname : String) {block}
        #note: initialize a module using specific dll
        def initialize(dllname, &block) 
            @dll = HEG::FFI::dlopen(dllname)
            @funcs = {}
            puts self.instance_variables
            instance_exec &block if block_given?
        end

        #[DOCUMENT]
        #method: define {block}
        #note: Define methods in block
        alias :define :instance_exec

        #type alias
        def int32 
            TYPE_INT32
        end 
        alias :int :int32
        def int64 
            TYPE_INT64
        end 
        def float 
            TYPE_FLOAT
        end 
        def double 
            TYPE_DOUBLE
        end
        def cstr 
            TYPE_STRING
        end
        def vptr 
            TYPE_VOIDP
        end

        def func(name, return_type, arg_type)
            @funcs[name.to_sym] = Function.new(@dll.addrof(name.to_s) ,return_type, arg_type.to_a)
        end

        def method_missing(name, *arg, &block) 
            #puts name
            f = @funcs[name.to_sym]
            if !f 
                raise NoMethodError, "C function #{name.to_s} was not defined." 
                return 0
            end
            f.call(*arg)
        end

        def close 
            @funcs = {}
            @dll.close
        end
    end
end