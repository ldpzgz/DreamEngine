#ifndef _AABB_H_
#define _AABB_H_
#include "Rect.h"
class AABB {
public:
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
	std::array<float, 6> xyz{ 0,0,0,0,0,0 };
};
#endif