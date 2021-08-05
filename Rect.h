#ifndef _RECT_H_
#define _RECT_H_

template<typename T>
class Rect {
public:
	Rect(T x1, T y1, T w, T h) :
		x(x1),y(y1),width(w),height(h)
	{

	}

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
	Color(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char a1) :r(r1), g(g1), b(b1), a(a1) {

	}

	Color(unsigned char r1, unsigned char g1, unsigned char b1) :r(r1), g(g1), b(b1) {

	}

	Color() = default;

	unsigned char r{ 0 };
	unsigned char g{ 0 };
	unsigned char b{ 0 };
	unsigned char a{ 0 };
};

#endif