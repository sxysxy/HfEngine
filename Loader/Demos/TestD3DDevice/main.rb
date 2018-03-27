device = DX::D3DDevice.new(DX::HARDWARE_DEVICE)
msgbox device.query_adapter_info
msgbox device.enum_adapters
