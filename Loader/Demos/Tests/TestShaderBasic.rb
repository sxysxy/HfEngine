require 'libcore'

vscode = <<VS
float4 main(float4 pos : POSITION) : SV_POSITION {
	return pos;
}
VS

device = DX::D3DDevice.new(DX::HARDWARE_DEVICE)
vs = DX::VertexShader::load_string device, vscode
msgbox vs