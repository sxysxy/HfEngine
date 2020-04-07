#encoding :utf-8
module HEG

    class Camera
        attr_reader :position, :front, :updir, :side, :angle_yaw, :angle_pitch

        def initialize
            reset
        end

        def reset 
            @position = [0.0, 0.0, 0.0]
            @front = [0.0, 0.0, 1.0]
            @updir = [0.0, 1.0, 0.0]
            @side = [1.0, 0.0, 0.0]
            @angle_pitch = 0.0
            @angle_yaw = 0.0
            #update
        end

        def position=(p)
            @position = p.map {|x| x.to_f}
        end

        def target 
            [@position[0] + @front[0], @position[1] + @front[1], @position[2] + @front[2]]
        end

        def move_forward(step)
            @position[0] += @front[0] * step
            @position[1] += @front[1] * step
            @position[2] += @front[2] * step  
        end
        def move_backward(step)
            move_forward(-step)
        end
        def move_left(step)
            @position[0] -= @side[0] * step
            @position[1] -= @side[1] * step
            @position[2] -= @side[2] * step
        end
        def move_right(step)
            move_left(-step)
        end
        def move_up(step)
            @position[1] += step
        end
        def move_down(step)
            @position[1] -= step
        end
        def yaw(angle)
            @angle_yaw += angle
            @angle_yaw = 179.9 if @angle_yaw <= -180.0
            @angle_yaw = -179.9 if @angle_yaw >= 180.0
            update
        end
        def pitch(angle)
            @angle_pitch += angle
            @angle_pitch = 89.9 if @angle_pitch >= 90.0
            @angle_pitch = -89.9 if @angle_pitch <= -90.0
            update
        end

        def update 
            y = radians(@angle_yaw)
            p = radians(@angle_pitch)
            @front[0] = Math.sin(y) * Math.cos(p)
            @front[1] = Math.sin(p)
            @front[2] = Math.cos(y) * Math.cos(p)
            @front = normalize(@front)

            @side[0] = Math.cos(y)
            @side[1] = 0.0
            @side[2] = -Math.sin(y)
            @side = normalize(@side)
        end 
        def radians(a)
            a * Math::PI / 180.0
        end
        def normalize(a)
            len = Math.sqrt(a[0]*a[0] + a[1]*a[1] + a[2]*a[2])
            a[0] /= len 
            a[1] /= len 
            a[2] /= len
            a 
        end
        private :update, :normalize, :radians
    end

end