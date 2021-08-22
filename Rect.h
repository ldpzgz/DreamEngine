#ifndef _RECT_H_
#define _RECT_H_
#include <array>
#include <string>
template<typename T>
class Rect {
public:
	Rect(T x1, T y1, T w, T h) :
		x(x1),y(y1),width(w),height(h)
	{

	}

	Rect() = default;

	Rect(const Rect& c) = default;

	/*Rect(const Rect& r):
		x(r.x), y(r.y), width(r.width), height(r.height)
	{
	}*/

	//template<typename U>
	//Rect(const Rect<U>& r) {
	//	x = r.x;
	//	y = r.y;
	//	width = r.width;
	//	height = r.height;
	//}

	bool isInside(T x1, T y1) {
		if (x1 >= x && x1 <= x + width && y1 >= y && y1 <= y + height){
			return true;
		}
		else {
			return false;
		}
	}
public:
	union {
		struct{ T x, y, width, height; };
		std::array<T, 4> rect;
	};
};

class Color {
public:
	Color(float r, float g, float b, float a) {
		rgba[0] = r;
		rgba[1] = g;
		rgba[2] = b;
		rgba[3] = a;
	}

	Color(float r, float g, float b) {
		rgba[0] = r;
		rgba[1] = g;
		rgba[2] = b;
	}

	Color() = default;

	Color(const Color& c) = default;

	float& operator[](int index) {
		return rgba[index];
	}

	union {
		struct { float r, g, b, a; };
		std::array<float, 4> rgba;
	};

	static bool parseColor(const std::string& value,Color& color) {
		if (value.empty() || value.size() > 9 || value[0]!='#') {
			return false;
		}
		int index = 0;
		int relIndex = 1 + 2 * index;
		bool hasError = false;
		while (index < 4 && relIndex < value.size()) {
			auto Rstr = value.substr(relIndex, 2);
			try {
				color[index] = (float)std::stoi(Rstr, nullptr, 16)/255.0f;
			}
			catch (.../*const logic_error& e*/) {
				hasError = true;
				break;
			}
			++index;
			relIndex = 1 + 2 * index;
		}
		return !hasError;
	}
};

#endif