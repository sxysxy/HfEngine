def require_lib(lib)
	require "./lib/#{lib}"
end

require_lib('HFWindow.rb')
require_lib('RenderPipeline.rb')
require_lib('DXInput')