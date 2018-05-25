require_lib 'Graphics2D'

G2D::Graphics.new("Renderer2D", 960, 720) { |g| #graphics 
	exit_flag = false
	g.on_exit = -> { exit_flag = true }

	renderer = G2D::Renderer.new(g)
	update_scene = -> {
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
		timer.await			#control fps
	}
	clean_up[]
}