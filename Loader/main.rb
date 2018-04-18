require 'libcore'
include DX
show_console
comp = HFSF::Compiler.compile_file(EXECUTIVE_DIRECTORY+'/testHFSFCode.rb')
#File.open("emm.txt", "w"){|f|f.print HFSF.loadsf(D3DDevice.new(HARDWARE_DEVICE), comp)}
sf = HFSF.loadsf(D3DDevice.new(HARDWARE_DEVICE), comp)[0]
msgbox sf.resource[:res].cbuffer[:param]

STDOUT.flush
system("pause")