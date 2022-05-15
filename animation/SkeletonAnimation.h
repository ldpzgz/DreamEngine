#pragma once
#include "../Animation.h"
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
class Skeleton;
/*
* one SkeletonAnimation link to one Skeleton,
*/
class SkeletonAnimation :public Animation {
public:
	explicit SkeletonAnimation(const std::string& name);

	SkeletonAnimation();

	//SkeletonAnimation(const SkeletonAnimation& other);

	void setDuration(int64_t dur) {
		mDuration = dur;
	}

	int64_t getDuration() {
		return mDuration;
	}

	void addPosKeyFrame(const std::string& nodeName, float* pTime, glm::vec3* pPos, int count);
	void addScaleKeyFrame(const std::string& nodeName, float* pTime, glm::vec3* pPos, int count);
	void addRotateKeyFrame(const std::string& nodeName, float* pTime, glm::quat* pPos, int count);

	void addPosKeyFrame(const std::string& nodeName,std::vector<KeyFrameVec3Time>& info) {
		if (!mpNodesPosKeyFrameInfo->try_emplace(nodeName, std::move(info)).second) {
			LOGE("addPosKeyFrame,the Node is already has pos keyframe info");
		}
	}
	void addScaleKeyFrame(const std::string& nodeName,std::vector<KeyFrameVec3Time>& info) {
		if (!mpNodesScaleKeyFrameInfo->try_emplace(nodeName, std::move(info)).second) {
			LOGE("addScaleKeyFrame,the Node is already has scale keyframe info");
		}
	}
	void addRotateKeyFrame(const std::string& nodeName,std::vector<KeyFrameQuatTime>& info) {
		if (!mpNodesRotateKeyFrameInfo->try_emplace(nodeName, std::move(info)).second) {
			LOGE("addRotateKeyFrame,the Node is already has rotate keyframe info");
		}
	}

	//void addAffectedNode(const std::string& name) {
	//	if (!mNameNodeMap.try_emplace(name,std::shared_ptr<Node>()).second) {
	//	}
	//}

	//void addAffectedNode(const std::string& name, const std::shared_ptr<Node>& pNode) {
	//	mNameNodeMap[name] = pNode;
	//}

	/*void addAffectedMesh(const std::shared_ptr<Mesh>& pMesh) {
		mAffectedMeshes.emplace_back(pMesh);
	}*/

	void setAffectedSkeleton(std::shared_ptr<Skeleton> ps) {
		mpSkeleton = ps;
	}

	void animate() override;
private:
	//void updateAffectedMesh();
	int64_t mDuration{ 0 };//in ms
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