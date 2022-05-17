#pragma once

//#include <glm/glm.hpp>
//#include <glm/vec2.hpp>           // vec3
//#include <glm/vec3.hpp>           // vec3

#include "animation/KeyFrame.h"
#include "Utils.h"
#include <vector>
#include <string_view>
//#include "Spline.h"
enum class AnimationType {
	NodeAnimation,
	SkeletonAnimation,
};
enum class InterpolationType {
	Linear,
	Step,
	CubicSpline
};

enum class AnimationState {
	Idle,
	Started,
	Paused
};
class Skeleton;
class Node;
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

	void setDuration(int64_t dur) {
		mDuration = dur;
	}

	int64_t getDuration() {
		return mDuration;
	}

	virtual void animate() {

	}

	virtual void setAffectedSkeleton(std::shared_ptr<Skeleton> ps) {

	}

	virtual void setTargetNode(const std::shared_ptr<Node> pNode) {

	}

	virtual void setPosKeyFrame(const char* nodeName, float* pTime, glm::vec3* pPos, int count, InterpolationType interType) {

	}
	virtual void setScaleKeyFrame(const char* nodeName, float* pTime, glm::vec3* pScale, int count, InterpolationType interType) {

	}
	virtual void setRotateKeyFrame(const char* nodeName, float* pTime, glm::quat* pRotate, int count, InterpolationType interType) {

	}

	virtual void addPosKeyFrame(const char* nodeName, std::vector<KeyFrameVec3Time>& info) {

	}
	virtual void addScaleKeyFrame(const char* nodeName, std::vector<KeyFrameVec3Time>& info) {

	}
	virtual void addRotateKeyFrame(const char* nodeName, std::vector<KeyFrameQuatTime>& info) {

	}

	static std::shared_ptr<Animation> createAnimation(AnimationType type,const std::string& name);
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
	int64_t mDuration{ 0 };//in ms
	TimerMil mTimer;
	AnimationState mState{ AnimationState::Idle };
	/*LoopMode mLoopMode{ LoopMode::None };
	InterpolateType mInterpolateType{InterpolateType::Cubic};
	
	tk::spline mSplineX;
	tk::spline mSplineY;*/
};
using AnimationSP = std::shared_ptr<Animation>;
