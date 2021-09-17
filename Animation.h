#ifndef _ANIMATION_H_
#define _ANIMATION_H_
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp> // vec3, vec4, ivec4, mat4
#include <glm/mat4x4.hpp>
#include <glm/mat4x3.hpp>
#include <glm/gtc/type_ptr.hpp> // value_ptr
#include "Rect.h"
#include "Utils.h"
#include <vector>
#include "Spline.h"

enum class LoopMode {
	None,
	ReturnBack,
	Loop
};

enum class InterpolateType {
	Linear,
	Cubic
};

class Animation {
public:
	Animation() = default;
	~Animation() = default;
	void start() {
		mTimer.start();
	}

	void stop() {
		mTimer.stop();
	}

	void pause() {
		mTimer.pause();
	}

	void resume() {
		mTimer.resume();
	}

	void setLoopModel(LoopMode mode) {
		mLoopMode = mode;
	}

	void setInterpolateType(InterpolateType type) {
		mInterpolateType = type;
	}
	/*
	* Vec2: x是要做插值的坐标，y是时间值
	*/
	void setControlPoints(const std::vector<Vec2>& cPoints);
	/*
	* Vec3: x，y是要做插值的坐标，z是时间值
	*/
	void setControlPoints(const std::vector<Vec3>& cPoints);

	float getCurX();

	Vec2 getCurXY();
protected:
	int64_t mAnimationTime{ 0 };
	int64_t mCurTime;
	LoopMode mLoopMode{ LoopMode::None };
	InterpolateType mInterpolateType{InterpolateType::Cubic};
	TimeCounterMil mTimer;
	tk::spline mSplineX;
	tk::spline mSplineY;
};

#endif