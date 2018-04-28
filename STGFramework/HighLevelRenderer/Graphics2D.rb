module Graphics
	def self.init
		if $config[:graphics].fps_vsync
			@fps = $device.query_monitor_info[:refresh_frequency]
		elsif $config[:graphics].fps
			@fps = Integer($config[:graphics].fps)
		else
			@fps = 60
		end
		
		@swap_chain = DX::SwapChain.new($device, $window)
		@swap_chain.set_fullscreen if $config[:graphics].fullscreen 
		
		@re = DX::RemoteRenderExecutive.new($device, @swap_chain, @fps)
	end
	def self.swap_chain
		return @swap_chain
	end
	def self.re
		return @re
	end
	def self.fps
		return @fps
	end
	def self.fps=(f)
		return @re.reset_fps(@fps = f)
	end
	def self.fullscreen(f = true)
		@swap_chain.set_fullscreen f
	end
	def self.shutdown
		@re.terminate
		@swap_chain.release
	end
	def self.rtt
		@swap_chain.rtt
	end
end