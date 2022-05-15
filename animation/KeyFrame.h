#pragma once

#include <glm/vec3.hpp>           // vec3
#include <glm/mat4x4.hpp>         // mat4
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_transform.hpp>

class KeyFrameVec3Time {
public:
	KeyFrameVec3Time(const glm::vec3& v, int64_t time) :
		vec(v),
		timeMs(time)
	{
	}
	glm::vec3 vec{ 0.0f,0.0f,0.0f };
	int64_t timeMs{ 0 };//∫¡√Î
};
class KeyFrameQuatTime {
public:
	KeyFrameQuatTime(const glm::quat& q, int64_t time) :
		rotate(q),
		timeMs(time)
	{
	}
	glm::quat rotate{ 0.0f,0.0f,0.0f,0.0f };
	int64_t timeMs{ 0 };//∫¡√Î
};

