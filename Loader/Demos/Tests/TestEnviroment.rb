require 'libcore'
show_console
puts "Running paths : \n#{EXECUTIVE_FILENAME} \n #{EXECUTIVE_DIRECTORY} \n #{__FILE__} \n #{File.dirname __FILE__}"
device = DX::D3DDevice.new(DX::HARDWARE_DEVICE)
puts "Adapter infomation:\n#{device.query_adapter_info}"
puts "Enmu Adapter:\n #{device.enum_adapters}"
STDOUT.flush
system("pause")