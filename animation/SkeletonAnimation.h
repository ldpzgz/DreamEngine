#pragma once
#include "core/Animation.h"
#include "KeyFrame.h"
#include <glm/vec3.hpp>           // vec3
#include <glm/mat4x4.hpp>         // mat4
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <unordered_map>
#include <vector>
#include <string>
class Mesh;
class Node;
/*
* one SkeletonAnimation link to one Skeleton,
*/
class SkeletonAnimation :public Animation {
public:
	explicit SkeletonAnimation(const std::string& name);

	SkeletonAnimation();

	~SkeletonAnimation() = default;

	void setPosKeyFrame(const std::string& nodeName, float* pTime, glm::vec3* pPos,
		int count, InterpolationType interType) override;
	void setScaleKeyFrame(const std::string& nodeName, float* pTime, glm::vec3* pPos,
		int count, InterpolationType interType) override;
	void setRotateKeyFrame(const std::string& nodeName, float* pTime, glm::quat* pPos,
		int count, InterpolationType interType) override;

	void addPosKeyFrame(const char* nodeName, std::vector<KeyFrameVec3Time>& info) override;
	void addScaleKeyFrame(const char* nodeName, std::vector<KeyFrameVec3Time>& info) override;
	void addRotateKeyFrame(const char* nodeName, std::vector<KeyFrameQuatTime>& info) override;

	void setAffectedSkeleton(std::shared_ptr<Skeleton> ps) override {
		mpSkeleton = ps;
	}

	void animate() override;
private:
	//void updateAffectedMesh();
	//the animation key frame info
	//nodeName-keyFrameInfo
	//each bone has name,and has a vector of pos,scale,rotate keyfram info.
	std::shared_ptr < std::unordered_map < std::string, std::vector<KeyFrameVec3Time> > > mpNodesPosKeyFrameInfo;
	std::shared_ptr < std::unordered_map < std::string, std::vector<KeyFrameVec3Time> > > mpNodesScaleKeyFrameInfo;
	std::shared_ptr < std::unordered_map < std::string, std::vector<KeyFrameQuatTime> > >mpNodesRotateKeyFrameInfo;
	//std::unordered_map<std::string, std::shared_ptr<Node>> mNameNodeMap;//not need
	//std::vector<std::shared_ptr<Mesh>> mAffectedMeshes;
	std::weak_ptr<Skeleton> mpSkeleton;
};