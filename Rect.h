#ifndef _RECT_H_
#define _RECT_H_

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
	T x, y, width, height;
};

class Color {
public:
	Color(float r1, float g1, float b1, float a1) :r(r1), g(g1), b(b1), a(a1) {

	}

	Color(float r1, float g1, float b1) :r(r1), g(g1), b(b1) {

	}

	Color() = default;

	Color(const Color& c) = default;

	float r{ 0.0f };
	float g{ 0.0f };
	float b{ 0.0f };
	float a{ 0.0f };
};

#endif