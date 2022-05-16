#include "Animation.h"
#include "animation/NodeAnimation.h"
#include "animation/SkeletonAnimation.h"

std::shared_ptr<Animation> Animation::createAnimation(AnimationType type,const std::string& name) {
	switch (type) {
	case AnimationType::NodeAnimation:
		return std::make_shared<NodeAnimation>(name);
		break;
	case AnimationType::SkeletonAnimation:
		return std::make_shared<SkeletonAnimation>(name);
		break;
	default:
		return {};
		break;
	}
}




//#include "Spline.h"           // vec3
//
//void Animation::setControlPoints(const std::vector<glm::vec2>& cPoints) {
//	std::vector<double> x;
//	std::vector<double> time;
//	for (auto& p : cPoints) {
//		x.push_back(p.x);
//		time.push_back(p.y);
//		mAnimationTime = p.y;
//	}
//	if (mInterpolateType == InterpolateType::Linear) {
//		mSplineX.set_points(time, x, tk::spline::linear);
//	}
//	else {
//		mSplineX.set_points(time, x, tk::spline::cspline);
//	}
//}
//
//void Animation::setControlPoints(const std::vector<glm::vec3>& cPoints) {
//	std::vector<double> x;
//	std::vector<double> y;
//	std::vector<double> time;
//	for (auto& p : cPoints) {
//		x.push_back(p.x);
//		y.push_back(p.y);
//		time.push_back(p.z);
//		mAnimationTime = p.z;
//	}
//	if (mInterpolateType == InterpolateType::Linear) {
//		mSplineX.set_points(time, x, tk::spline::linear);
//		mSplineY.set_points(time, y, tk::spline::linear);
//	}
//	else {
//		mSplineX.set_points(time, x, tk::spline::cspline);
//		mSplineY.set_points(time, y, tk::spline::cspline);
//	}
//}
//
//float Animation::getCurX() {
//	auto curTime = mTimer.elapseFromStart();
//	if (mLoopMode == LoopMode::None) {
//		if (curTime >= mAnimationTime) {
//			return mSplineX(mAnimationTime);
//		}
//		else {
//			return mSplineX(curTime);
//		}
//	}
//	else if (mLoopMode == LoopMode::Loop) {
//		curTime %= mAnimationTime;
//		return mSplineX(curTime);
//	}
//	else if (mLoopMode == LoopMode::ReturnBack) {
//		auto n = curTime / mAnimationTime;
//		curTime %= mAnimationTime;
//		if (n & 0x00000001) {
//			return mSplineX(mAnimationTime-curTime);
//		}
//		else {
//			return mSplineX(curTime);
//		}
//	}
//}
//
//glm::vec2 Animation::getCurXY() {
//	auto curTime = mTimer.elapseFromStart();
//	glm::vec2 ret(0.0f,0.0f);
//	if (mLoopMode == LoopMode::None) {
//		if (curTime >= mAnimationTime) {
//			ret.x = mSplineX(mAnimationTime);
//			ret.y = mSplineY(mAnimationTime);
//		}
//		else {
//			ret.x = mSplineX(curTime);
//			ret.y = mSplineY(curTime);
//		}
//	}
//	else if (mLoopMode == LoopMode::Loop) {
//		curTime %= mAnimationTime;
//		ret.x = mSplineX(curTime);
//		ret.y = mSplineY(curTime);
//	}
//	else if (mLoopMode == LoopMode::ReturnBack) {
//		auto n = curTime / mAnimationTime;
//		curTime %= mAnimationTime;
//		if (n & 0x00000001) {
//			ret.x = mSplineX(mAnimationTime - curTime);
//			ret.y = mSplineY(mAnimationTime - curTime);
//		}
//		else {
//			ret.x = mSplineX(curTime);
//			ret.y = mSplineY(curTime);
//		}
//	}
//	return ret;
//}