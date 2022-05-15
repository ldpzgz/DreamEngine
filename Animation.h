#pragma once

//#include <glm/glm.hpp>
//#include <glm/vec2.hpp>           // vec3
//#include <glm/vec3.hpp>           // vec3

//#include "Rect.h"
#include "Utils.h"
#include <vector>
//#include "Spline.h"

enum class AnimationState {
	Idle,
	Started,
	Paused
};

class Animation {
public:
	Animation() = default;
	explicit Animation(const std::string& name) :
		mName(name) {

	}
	virtual ~Animation() = default;

	void setName(const std::string& name) {
		mName = name;
	}

	const std::string& getName() {
		return mName;
	}

	void start() {
		mTimer.start();
		mState = AnimationState::Started;
	}

	void stop() {
		mTimer.stop();
		mState = AnimationState::Idle;
	}

	void pause() {
		mTimer.pause();
		mState = AnimationState::Paused;
	}

	void resume() {
		mTimer.resume();
		mState = AnimationState::Started;
	}

	AnimationState getStatus() {
		return mState;
	}

	bool isStarted() {
		return mState == AnimationState::Started;
	}

	virtual void animate() {

	}
	/*void setLoopModel(LoopMode mode) {
		mLoopMode = mode;
	}

	void setInterpolateType(InterpolateType type) {
		mInterpolateType = type;
	}*/
	/*
	* glm::vec2: x是要做插值的坐标，y是时间值
	*/
	//void setControlPoints(const std::vector<glm::vec2>& cPoints);
	/*
	* Vec3: x，y是要做插值的坐标，z是时间值
	*/
	//void setControlPoints(const std::vector<glm::vec3>& cPoints);

	//float getCurX();

	//glm::vec2 getCurXY();
protected:
	std::string mName;
	int64_t mAnimationTime{ 0 };
	int64_t mCurTime{0};
	TimerMil mTimer;
	AnimationState mState{ AnimationState::Idle };
	/*LoopMode mLoopMode{ LoopMode::None };
	InterpolateType mInterpolateType{InterpolateType::Cubic};
	
	tk::spline mSplineX;
	tk::spline mSplineY;*/
};
using AnimationSP = std::shared_ptr<Animation>;
