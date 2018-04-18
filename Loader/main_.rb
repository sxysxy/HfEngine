require 'libcore'

show_console
comp = HFSF::Compiler.compile_file(EXECUTIVE_DIRECTORY+'/testHFSFCode.rb')
File.open("emm.txt", "w"){|f|f.print comp.row_data}
STDOUT.flush
system("pause")