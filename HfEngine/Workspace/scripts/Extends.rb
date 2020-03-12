require 'FFI'
module HEG 
    module EXT 
        COMDLG = HEG::FFI::Module.new("comdlg32.dll") {
            func :GetOpenFileNameA, int, [cstr]
        }
        OPENFILENAMEA = HEG::FFI::CStruct.new {
            int64 :struct_size
            vptr :owner, :instance
            vptr :filter
            vptr :custom_fiter
            int :max_cust_filter, :filter_index
            vptr :pfile
            int64 :max_file_num
            vptr :file_title
            int64 :max_file_title_num
            vptr :dir, :title 
            int :flag
            #do not use
            int :i1
            vptr :v1, :v2, :v3, :v4
            vptr :v5
            int64 :l1
        }
    end

    module Filebox
        LOAD = 0
        SAVE = 1
        NO_FILTER = [["All files(*.*)", "*.*"]]
        def self.show(**keywords)
            mode = keywords[:mode] || LOAD
            filter = keywords[:filter] || NO_FILTER
            flag = 0
            if mode == LOAD 
                flag = 0x800 | 0x10000 | 0x80000
            elsif mode == SAVE 
                flag = 0x800 | 0x80000
            else
                raise ArgumentError, "invalid mode(Other than LOAD or SAVE)" 
            end
            os = EXT::OPENFILENAMEA.new 
            os.zero_all
            os[:struct_size] = 152
            os[:filter] = str_ptr(filter.flatten.join("\0") + "\0\0")
            os[:dir] = str_ptr(keywords[:init_dir] || ".")
            os[:title] = str_ptr(keywords[:title] || "")
            os[:max_file_num] = 256
            os[:flag] = flag
            buf = "\0" * 255
            os[:pfile] = str_ptr(buf)
            p = os.pack 
            #msgbox "", "#{p.bytes.size}\n#{p.bytes}"
            EXT::COMDLG.GetOpenFileNameA(os.pack)
            #return buf.split("\0").map! {|s| local_to_u8(s).gsub!("\\", "/")}
            return buf.split("\0").map! {|s| local_to_u8(s)}
        end

    end

end