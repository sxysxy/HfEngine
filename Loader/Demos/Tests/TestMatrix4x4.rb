show_console

m = MathTool::Matrix4x4.new
puts "An indentity matrix : #{m.array_data} "
m[0, 1] = 2.0
m[1, 3] = 3.0
puts "Set some element : #{m.array_data} "
puts "m[1][3] = #{m[1, 3]}"
m.tranpose!
puts "after tranpose : #{m.array_data} "

puts "\n==============================================\n"
w = MathTool.move(-2.0, 1.0, 1.0)
puts "Move(-2.0, 1.0, 1.0) #{w.array_data}"
w *= MathTool.zoom(0.5, 0.5, 0.5)
puts "Then Zoom(0.5, 0.5, 0.5) #{w.array_data}"
w *= MathTool.rotate_round([1.0, 0.0, 1.0, 0.0], MathTool::PIDIV4)
puts "Then rotate round(1.0, 0.0, 1.0, 0.0), angle 0.25 * pi Thw world matrix : #{w.array_data}"

v = MathTool.lookat([-4.0, 2.0, 1.0, 0.0], [0.0]*4)
puts "Lookat(-4.0, 2.0, 1.0), target position is (0.0, 0.0, 0.0) : #{v.array_data}"

p = MathTool.perspective(MathTool::PIDIV4, 1.0, 0.1, 1000.0)
puts "Perspective(0.25 * pi, 1.0, 0.1, 1000.0, matrix : #{p.array_data}"
puts "\n==============================================\n"
wvp = w*v*p
puts "WVP matrix : #{wvp.array_data}"
wvp.tranpose!
puts "WVP matrix T(can be used in hlsl) : #{wvp.array_data}"

STDOUT.flush
system("pause")
