def require_lib(lib)
	require "./libruby/#{lib}"
end

require_lib('HFWindow.rb')
require_lib('RenderPipeline.rb')
require_lib('DXInput')
require_lib('RSDSL')