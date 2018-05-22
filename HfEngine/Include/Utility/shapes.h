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
        union {
            float r;
            float red;
        };
        union {
            float g;
            float green;
        };
        union {
            float b;
            float blue;
        };
        union {
            float a;
            float alpha;
        };
		Color() { r = g = b = a = 0.0f; }
		Color(float _r, float _g, float _b, float _a) {
			r = _r, g = _g, b = _b, a = _a;
		}
	};
    using Color32 = Color;

    struct Color8 {
        union {
            union {
                unsigned char r;
                unsigned char red;
            };
            union {
                unsigned char g;
                unsigned char green;
            };
            union {
                unsigned char b;
                unsigned char blue;
            };
            union {
                unsigned char a;
                unsigned char alpha;
            };
            unsigned int rgba;
        };
        Color8() {r = b = g = a = 0;}
        Color8(unsigned int c) {
            rgba =  c;
        }
        Color8(unsigned char _r, unsigned char _g, unsigned char _b, unsigned char _a) {
            r = _r, g = _g, b = _b, a = _a;
        }
    };

}