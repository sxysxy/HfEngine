#encoding: utf-8
require 'Graphics2D'
require 'Font'

G2D::Graphics.init("DrawFont", 400, 400) {|g| 
    exit_flag = false

    g.on_exit = ->{exit_flag = true}

    renderer = G2D::Renderer.new()
    font = Font.new("simfang.ttf", 24);  #宋体
    text_sprite = G2D::Sprite.new(font.render_texture2d(g.device, "字体", HFColorRGBA(1.0, 1.0, 1.0, 1.0))) #白色字“字体”

    timer = FPSTimer.new(60)
    while !exit_flag
        g.update
        renderer.clear

        renderer.draw_sprite(text_sprite)
        renderer.render
        timer.await
    end
}