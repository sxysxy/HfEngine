require_lib 'Graphics2D'

G2D::Graphics.new("Renderer2D", 960, 720) { |g| #graphics 
	exit_flag = false
	g.on_exit = -> { exit_flag = true }

	renderer = G2D::Renderer.new(g)
	update_scene = -> {
		renderer.clear
		renderer.draw_rect(HFRect(100, 100, 200, 100), 0.3, HFColorRGBA(1.0, 0.0, 1.0, 1.0))
		
		s = G2D::Sprite.new(g, DX::Texture2D.new(g.device, 400, 400))
		s.x = 400
		s.y = 300
		renderer.set_target(DX::RTT.new(s.texture))
		renderer.clear
		renderer.draw_rect(HFRect(300, 300, 100, 100), 0.4, HFColorRGBA(0.0, 1.0, 1.0, 1.0))
		renderer.set_target(g.rtt)
		renderer.draw_sprite(s)
		s.texture.release
		
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