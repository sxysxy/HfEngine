#encoding :utf-8
module TextureCache
	RESOURCE_PATH = File.join(File.dirname(__FILE__), "Resources/")
	
	def self.load(filename)
		@textures ||= {}
		
		fn = RESOURCE_PATH + filename
		@textures[fn] or DX::Texture2D.new($device, fn)
	end
	
end
