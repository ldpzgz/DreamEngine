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
		rgba[3] = 1.0f;
	}

	Color(unsigned char r, unsigned char g, unsigned char b, unsigned char a) {
		rgba[0] = r / 255.0f;
		rgba[1] = g / 255.0f;
		rgba[2] = b / 255.0f;
		rgba[3] = a / 255.0f;
	}

	Color(unsigned char r, unsigned char g, unsigned char b) {
		rgba[0] = r / 255.0f;
		rgba[1] = g / 255.0f;
		rgba[2] = b / 255.0f;
		rgba[3] = 1.0f;
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

	union {
		struct { float r, g, b, a; };
		std::array<float, 4> rgba;
	};

	static bool parseColor(const std::string& value, Color& color);
};

template<typename T>
struct Vector3 {
	Vector3():x(0),y(0),z(0) {
	}

	Vector3(T x, T y, T z) {
		data[0] = x;
		data[1] = y;
		data[2] = z;
	}
	Vector3(const Vector3<T>& other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}

	void operator=(const Vector3<T>& other) {
		x = other.x;
		y = other.y;
		z = other.z;
	}

	Vector3<T> operator*(T a) {
		return Vector3<T>(a*x, a*y, a*z);
	}

	void operator*=(T a) {
		x *= a;
		y *= a;
		z *= a;
	}

	Vector3<T> operator+(const Vector3<T>& other) {
		return Vector3<T>(x + other.x, y + other.y, z + other.z);
	}

	Vector3<T> operator-(const Vector3<T>& other) {
		return Vector3<T>(x - other.x, y - other.y, z - other.z);
	}

	Vector3<T> operator+=(const Vector3<T>& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		return *this;
	}

	Vector3<T> operator-=(const Vector3<T>& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		return *this;
	}

	T distance(const Vector3<T>& other) const{
		return sqrt(pow(x - other.x, 2) + pow(y - other.y, 2) + pow(z - other.z, 2));
	}

	template<typename T>
	friend Vector3<T> operator+(const Vector3<T>& A, const Vector3<T>& B);

	template<typename T>
	friend Vector3<T> operator-(const Vector3<T>& A, const Vector3<T>& B);

	template<typename T>
	friend Vector3<T> operator*(T a, const Vector3<T>& v);

	union {
		struct { T x, y, z; };
		std::array<T, 3> data;
	};
};

template<typename T>
Vector3<T> operator+(const Vector3<T>& a, const Vector3<T>& b) {
	return Vector3<T>(a.x+b.x,a.y+b.y,a.z+b.z);
}

template<typename T>
Vector3<T> operator-(const Vector3<T>& a, const Vector3<T>& b) {
	return Vector3<T>(a.x - b.x, a.y - b.y, a.z - b.z);
}

template<typename T>
Vector3<T> operator*(T a, const Vector3<T>& v) {
	return Vector3<T>(a * v.x, a * v.y, a * v.z);
}

template<typename T>
struct Vector2 {
	Vector2() :x(0), y(0) {
	}

	Vector2(T x, T y) {
		data[0] = x;
		data[1] = y;
	}
	Vector2(const Vector3<T>& other) {
		x = other.x;
		y = other.y;
	}

	void operator=(const Vector2<T>& other) {
		x = other.x;
		y = other.y;
	}

	Vector2<T> operator*(T a) {
		return Vector2<T>(a * x, a * y);
	}

	void operator*=(T a) {
		x *= a;
		y *= a;
	}

	Vector2<T> operator+(const Vector2<T>& other) {
		return Vector2<T>(x + other.x, y + other.y);
	}

	Vector2<T> operator-(const Vector2<T>& other) {
		return Vector2<T>(x - other.x, y - other.y);
	}

	Vector2<T> operator+=(const Vector2<T>& other) {
		x += other.x;
		y += other.y;
		return *this;
	}

	Vector2<T> operator-=(const Vector2<T>& other) {
		x -= other.x;
		y -= other.y;
		return *this;
	}

	T distance(const Vector2<T>& other) const{
		return sqrt(pow(x - other.x, 2) + pow(y - other.y, 2));
	}

	template<typename T>
	friend Vector2<T> operator+(const Vector2<T>& A, const Vector2<T>& B);

	template<typename T>
	friend Vector2<T> operator-(const Vector2<T>& A, const Vector2<T>& B);

	template<typename T>
	friend Vector2<T> operator*(T a, const Vector2<T>& v);

	union {
		struct { T x, y; };
		std::array<T, 2> data;
	};
};

template<typename T>
Vector2<T> operator+(const Vector2<T>& a, const Vector2<T>& b) {
	return Vector2<T>(a.x + b.x, a.y + b.y);
}

template<typename T>
Vector2<T> operator-(const Vector2<T>& a, const Vector2<T>& b) {
	return Vector2<T>(a.x - b.x, a.y - b.y);
}

template<typename T>
Vector2<T> operator*(T a, const Vector2<T>& v) {
	return Vector2<T>(a * v.x, a * v.y);
}



template<typename T>
struct Vector4 {
	Vector4() :x(0), y(0), z(0),w(0){
	}

	Vector4(T a, T b, T c,T d) :x(a), y(b), z(c), w(d) {
	}

	Vector4(const Vector4<T>& other) {
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
	}

	void operator=(const Vector4<T>& other) {
		x = other.x;
		y = other.y;
		z = other.z;
		w = other.w;
	}

	void operator*=(T a) {
		x *= a;
		y *= a;
		z *= a;
		w *= a;
	}

	Vector4<T> operator+(const Vector4<T>& other) {
		return Vector4<T>(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	Vector4<T> operator-(const Vector4<T>& other) {
		return Vector4<T>(x - other.x, y - other.y, z - other.z, w - other.w);
	}

	Vector4<T> operator+=(const Vector4<T>& other) {
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;
		return *this;
	}

	Vector4<T> operator-=(const Vector4<T>& other) {
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;
		return *this;
	}

	T distance(const Vector4<T>& other) const{
		return sqrt(pow(x - other.x, 2) + pow(y - other.y, 2) + pow(z - other.z, 2) + pow(w - other.w, 2));
	}

	template<typename T>
	friend Vector4<T> operator+(const Vector4<T>& A, const Vector4<T>& B);

	template<typename T>
	friend Vector4<T> operator-(const Vector4<T>& A, const Vector4<T>& B);

	template<typename T>
	friend Vector4<T> operator*(T a,const Vector4<T>& v);

	union {
		struct { T x, y, z,w; };
		std::array<T, 4> data;
	};
};

template<typename T>
Vector4<T> operator+(const Vector4<T>& a, const Vector4<T>& b) {
	return Vector4<T>(a.x + b.x, a.y + b.y, a.z + b.z,a.w+b.w);
}

template<typename T>
Vector4<T> operator-(const Vector4<T>& a, const Vector4<T>& b) {
	return Vector4<T>(a.x - b.x, a.y - b.y, a.z - b.z, a.w - b.w);
}

template<typename T>
Vector4<T> operator*(T a,const Vector4<T>& v) {
	return Vector4<T>(a * v.x, a * v.y, a * v.z, a * v.w);
}

using Vec3 = Vector3<float>;
using Vec3ui = Vector3<unsigned int>;
using Vec4 = Vector4<float>;
using Vec2 = Vector2<float>;

#endif