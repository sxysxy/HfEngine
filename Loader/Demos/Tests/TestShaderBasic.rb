require 'libcore'

vscode = RSDSL.generate {
	struct(:vs_output){
		declare :pos, :float4, :SV_POSITION
		declare :color, :float4, :COLOR
	}
	defunc(:main, :vs_output, [:pos, :float4, :POSITION], [:color, :float4, :COLOR]){
		dvar :opt, :vs_output
		svar "opt.ops", "pos"
		svar "opt.color", "color"
		creturn {p "opt"}
	}
}
msgbox vscode