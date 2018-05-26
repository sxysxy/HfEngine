require_lib 'Graphics2D'

G2D::Graphics.new("Renderer2D", 960, 720) { |g| #graphics 
	exit_flag = false
	g.on_exit = -> { exit_flag = true }

	renderer = G2D::Renderer.new(g)
	time = 0
	update_scene = -> {
		renderer.clear
		renderer.draw_rect(HFRect(100, 100, 500, 500), 0.3, HFColorRGBA(1.0, 0.0, 1.0, 1.0))
		
		#Test Sprite
		s = G2D::Sprite.new(g, DX::Texture2D.new(g.device, 100, 100))
		s.x = time % g.window.width
		s.y = time % g.window.height
		s.opacity = [(time % 255) / 255.0 + 0.5, 1.0].min
		#And RTT(Render to texture)
		renderer.set_target(DX::RTT.new(s.texture))
		renderer.clear
		renderer.draw_rect(HFRect(0, 0, 100, 100), 0.4, HFColorRGBA((time % 255) / 255.0, ((time+30) % 255) / 255.0, ((time+100) % 255) / 255.0, 1.0))
		renderer.target.release
		#Then render to backbuffer
		renderer.set_target(g.rtt)
		renderer.draw_sprite(s)
		s.texture.release
		
		time += 1
		renderer.render
	}
	
	clean_up = -> {
		renderer.release
		g.shutdown
	}
	
	timer = FPSTimer.new(60)
	loop {
		g.update 		    #update the window
		break if exit_flag  #get exit flag
		update_scene[]      
		timer.await			#control logical fps
	}
	clean_up[]
}