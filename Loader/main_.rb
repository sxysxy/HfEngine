require 'libcore'

show_console
comp = HFSF::Compiler.compile_file(EXECUTIVE_DIRECTORY+'/testHFSFCode.rb')
comp.save_file("emm.txt")
STDOUT.flush
system("pause")