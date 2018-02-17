class HFWindow
	attr_accessor :handlers
	
	def set_handler(sym, &callback)
		@handlers[sym] = callback
	end
	
	def call_handler(sym, *arg)
		if @handlers[sym]
			@handlers[sym].call(*arg)
			return true
		end
		return false
	end
	
end