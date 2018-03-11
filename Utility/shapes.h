#pragma once

namespace Utility{

	struct Point {
		int x, y;
		Point() { x = y = 0; }
	};

	struct Rect {
		int x, y; 
        union {
            int w;
            int width;
        };
        union {
            int h;
            int height;
        };
		Rect() { x = y = w = h = 0; }
		Rect(int _x, int _y, int _w, int _h) {
			x = _x, y = _y, w = _w, h = _h;
		}
	};

	struct Circle {
		Point center;
		int r;
		Circle() { r = 0; }
	};

	struct Color {
		float r, g, b, a;
		Color() { r = g = b = a = 0.0f; }
		Color(float _r, float _g, float _b, float _a) {
			r = _r, g = _g, b = _b, a = _a;
		}
	};

}