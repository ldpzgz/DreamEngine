#ifndef _AABB_H_
#define _AABB_H_
#include <utility>
template<class T>
class Vector3;
class AABB {
public:
	AABB()=default;
	AABB(float x1, float x2, float y1, float y2, float z1, float z2) :
		xyz{ x1,x2,y1,y2,z1,z2 }
	{

	}
	float xmin() {
		return xyz[0];
	}
	float xmax() {
		return xyz[1];
	}
	float ymin() {
		return xyz[2];
	}
	float ymax() {
		return xyz[3];
	}
	float zmin() {
		return xyz[4];
	}
	float zmax() {
		return xyz[5];
	}
	Vec3 center() {
		return Vec3((xyz[0] + xyz[1]) / 2.0f, (xyz[2] + xyz[3]) / 2.0f, (xyz[4] + xyz[5]) / 2.0f);
	}
	Vec3 length() {
		return Vec3((xyz[1] - xyz[0]) / 2.0f, (xyz[3] - xyz[2]) / 2.0f, (xyz[5] - xyz[4]) / 2.0f);
	}
	AABB operator+(const AABB& other) {
		return AABB(std::min<float>(xyz[0],other.xyz[0]),
			std::max<float>(xyz[1], other.xyz[1]),
			std::min<float>(xyz[2], other.xyz[2]),
			std::max<float>(xyz[3], other.xyz[3]),
			std::min<float>(xyz[4], other.xyz[4]),
			std::max<float>(xyz[5], other.xyz[5]));
	}
	AABB operator+=(const AABB& other) {
		xyz[0] = std::min<float>(xyz[0], other.xyz[0]);
		xyz[1] = std::max<float>(xyz[1], other.xyz[1]);
		xyz[2] = std::min<float>(xyz[2], other.xyz[2]);
		xyz[3] = std::max<float>(xyz[3], other.xyz[3]);
		xyz[4] = std::min<float>(xyz[4], other.xyz[4]);
		xyz[5] = std::max<float>(xyz[5], other.xyz[5]);
		return *this;
	}
	std::array<float, 6> xyz{ 0,0,0,0,0,0 };
};
#endif