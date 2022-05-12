#ifndef _COLOR_H_
#define _COLOR_H_
#include <array>
#include <string>

class Color {
public:
	Color(float r1, float g1, float b1, float a1):
		r(r1),g(g1),b(b1),a(a1)
	{
	}

	Color(float r1, float g1, float b1):
		r(r1), g(g1), b(b1), a(1.0f)
	{
	}

	Color(unsigned char r1, unsigned char g1, unsigned char b1, unsigned char a1):
		r(r1/255.0f), g(g1 / 255.0f), b(b1 / 255.0f), a(a1 / 255.0f)
	{
	}

	Color(unsigned char r1, unsigned char g1, unsigned char b1) :
		r(r1 / 255.0f), g(g1 / 255.0f), b(b1 / 255.0f), a(1.0f)
	{
	}

	Color() :r(0.0f),g(0.0f),b(0.0f),a(1.0f){
	};

	Color(const Color& c) = default;

	float& operator[](int index) {
		return rgba[index];
	}

	bool operator==(const Color& c) {
		return (r == c.r && g == c.g && b == c.b && a == c.a);
	}

	bool operator!=(const Color& c) {
		return (r != c.r || g != c.g || b != c.b || a != c.a);
	}

	bool isZero() const{
		if (r == 0.0f && g == 0.0f && b == 0.0f) {
			return true;
		}
		return false;
	}

	void setColor4(float* pc) {
		r = pc[0];
		g = pc[1];
		b = pc[2];
		a = pc[3];
	}

	void setColor3(float* pc) {
		r = pc[0];
		g = pc[1];
		b = pc[2];
	}

	union {
		struct { float r, g, b, a; };
		std::array<float, 4> rgba;
	};

	static bool parseColor(const std::string& value, Color& color);
};
#endif