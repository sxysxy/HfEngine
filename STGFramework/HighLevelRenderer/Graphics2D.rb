module Graphics
	def self.init
		@fps = $config[:graphics].fps or 60
		
		@swap_chain = DX::SwapChain.new($device, $window)
		@swap_chain.set_fullsreen if $config[:graphics].fullscreen 
		
		@re = DX::RemoteRenderExecutive.new($device, @swap_chain, @fps)
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
		@swap_chain.set_fullsreen f
	end
end