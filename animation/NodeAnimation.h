#pragma once

#include "../Animation.h"
#include "KeyFrame.h"
class Node;
class NodeAnimation : public Animation{
public:
	using Animation::Animation;
	~NodeAnimation() = default;

	void setPosKeyFrame(const char* nodeName, float* pTime,
		glm::vec3* pPos, int count, InterpolationType interType) override;

	void setScaleKeyFrame(const char* nodeName,
		float* pTime, glm::vec3* pScale, int count, InterpolationType interType) override;

	void setRotateKeyFrame(const char* nodeName,
		float* pTime, glm::quat* pRotate, int count, InterpolationType interType) override;

	void setTargetNode(const std::shared_ptr<Node> pNode) override {
		mpTargetNode = pNode;
	}

	void animate() override;
private:
	/*
	* there are three kinds of interpolation type:linear(1),step(2),cubic_spline(3)
	*/
	std::shared_ptr<Node> mpTargetNode;
	std::unique_ptr < std::vector<KeyFrameVec3Time> > mpPosKeyFrames;
	InterpolationType mPosInterpolationType{ InterpolationType::Linear };
	std::unique_ptr < std::vector<KeyFrameVec3Time> > mpScaleKeyFrames;
	InterpolationType mScaleInterpolationType{ InterpolationType::Linear };
	std::unique_ptr < std::vector<KeyFrameQuatTime> >mpRotateKeyFrames;
	InterpolationType mRotateInterpolationType{ InterpolationType::CubicSpline };
};