#ifndef _ANIMATION_H_
#define _ANIMATION_H_
//#include <glm/glm.hpp>
#include <glm/vec2.hpp>           // vec3
#include <glm/vec3.hpp>           // vec3

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
	* glm::vec2: x是要做插值的坐标，y是时间值
	*/
	void setControlPoints(const std::vector<glm::vec2>& cPoints);
	/*
	* Vec3: x，y是要做插值的坐标，z是时间值
	*/
	void setControlPoints(const std::vector<glm::vec3>& cPoints);

	float getCurX();

	glm::vec2 getCurXY();
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