#encoding: utf-8
module G2D
	class Graphics
		attr_reader :window
		attr_reader :device
		attr_reader :swapchain
		attr_reader :render_exec
		attr_accessor :on_exit
		
		alias :re :render_exec

		def initialize(title, width, height, fps = :vsync)
			@window = HFWindow.new(title, width, height)
			@window.show
			@window.set_handler(:on_closed) {@on_exit.call if @on_exit.respond_to?(:call)}
			
			@device = DX::D3DDevice.new
			@swapchain = DX::SwapChain.new(@device, @window)
			if fps == :vsync
				@render_exec = DX::RemoteRenderExecutive.new(@device, @swapchain, @device.query_monitor_info[:refresh_frequency])
			elsif fps.is_a?(Integer)
				@render_exec = DX::RemoteRenderExecutive.new(@device, @swapchain, fps)
			else
				raise ArgumentError, "Unknown fps #{fps}"
			end
			
			yield(self) if block_given?
		end
		
		def update
			process_message
		end
		
		def shutdown
			@render_exec.terminate
			[@render_exec, @swapchain, @device].each &:release
		end
		
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
		def lock
			@render_exec.lock
		end
		def unlock
			@render_exec.unlock
		end
		def lock_exec
			lock
			yield if block_given?
			unlock
		end

	end
end