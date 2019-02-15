#encoding: utf-8
module G2D
	class Graphics

		class << self
			attr_reader :window
			attr_reader :device
			attr_reader :swapchain
			attr_reader :render_exec
			alias :re :render_exec
			attr_accessor :on_exit
		end	

		def self.init(title, width, height, fps = :vsync)
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
			return self
		end
		
		def self.update
			process_message
		end
		
		def self.shutdown
			@render_exec.terminate
			[@render_exec, @swapchain, @device].each &:release
		end
		
		def self.width
			@window.width
		end
		def self.height
			@window.height
		end
		def self.rtt
			@swapchain.rtt
		end
		def self.backbuffer
			@swapchain.backbuffer
		end
		def self.lock
			@render_exec.lock
		end
		def self.unlock
			@render_exec.unlock
		end
		def self.lock_exec
			lock
			yield if block_given?
			unlock
		end
		def self.fullscreen
			@swapchain.set_fullscreen true
		end
		def self.windowed_screen
			@swapchain.set_fullscreen false
		end

		def self.delay(time)
			time.times {self.update}
		end

	end
end