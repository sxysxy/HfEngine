require 'libcore'

show_console
print HFSF::Compiler.compile_file(EXECUTIVE_DIRECTORY+'/testHFSFCode.rb')
puts ""
STDOUT.flush
system("pause")