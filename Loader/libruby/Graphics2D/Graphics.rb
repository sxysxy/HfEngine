#encoding: utf-8
module G2D
	class Graphics
		attr_reader :window
		attr_reader :device
		attr_reader :swapchain
		attr_reader :render_exec
		attr_accessor :on_exit
		
		def initialize(title, width, height, fps = 60, &block)
			@window = HFWindow.new(title, width, height)
			@window.show
			@window.set_handler(:on_closed) {@on_exit.call if @on_exit.respond_to?(:call)}
			
			@device = DX::D3DDevice.new
			@swapchain = DX::SwapChain.new(@device, @window)
			@render_exec = DX::RemoteRenderExecutive.new(@device, @swapchain, fps)
			
			exec &block
		end
		
		def update
			process_message
		end
		
		def shutdown
			@render_exec.terminate
			[@render_exec, @swapchain, @device].each &:release
		end
		
		def exec(&block)
			raise "No block given" if !block
			block.call(self)
		end
		private :exec
		
		def width
			@window.width
		end
		def height
			@window.height
		end
		def rtt
			@swapchain.rtt
		end
		def backbuffer
			@swapchain.backbuffer
		end
	end
end