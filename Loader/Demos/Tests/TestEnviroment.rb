require 'libcore'
msgbox "#{EXECUTIVE_FILENAME} \n #{EXECUTIVE_DIRECTORY} \n #{__FILE__} \n #{File.dirname __FILE__}"
device = DX::D3DDevice.new(DX::HARDWARE_DEVICE)
msgbox device.query_adapter_info
msgbox device.enum_adapters
