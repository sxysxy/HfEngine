require 'libcore'

show_console
d = DX::Sampler.new.use_default.dump_description
x = DX::Sampler.new
x.load_description d
print HFSF::Compiler.compile_file(EXECUTIVE_DIRECTORY+'/testHFSFCode.rb').list
STDOUT.flush
system("pause")