class Sprite < G2D::Sprite
    def initialize(tex)
        super($graphics, tex)
    end

    #for being compatible with old interface
    alias :zoom_x= :scale_x=
    alias :zoom_y= :scale_y=
    
    def release_texture
        @texture.release
    end
end