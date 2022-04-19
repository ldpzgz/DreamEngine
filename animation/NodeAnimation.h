#pragma once
#include "../Animation.h"
#include <glm/vec3.hpp>           // vec3
#include <glm/mat4x4.hpp>         // mat4
#include <glm/ext/quaternion_float.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <unordered_set>
class Mesh;
class Node;
class NodeAnimation :public Animation {
public:
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

	NodeAnimation(const std::string& name, std::shared_ptr<Node> pRootNode) :
		mName(name),
		mpRootNode(pRootNode)
	{

	}

	void setName(const std::string& name) {
		mName = name;
	}

	const std::string& getName() {
		return mName;
	}

	void setDuration(int64_t dur) {
		mDuration = dur;
	}

	int64_t getDuration() {
		return mDuration;
	}

	void addPosKeyFrame(const std::string& nodeName,
		std::unique_ptr<std::vector<KeyFrameVec3Time>>& info) {
		if (!mNodesPosKeyFrameInfo.emplace(nodeName, std::move(info)).second) {
			LOGE("addPosKeyFrame,the Node is already has pos keyframe info");
		}
	}
	void addScaleKeyFrame(const std::string& nodeName,
		std::unique_ptr<std::vector<KeyFrameVec3Time>>& info) {
		if (!mNodesScaleKeyFrameInfo.emplace(nodeName, std::move(info)).second) {
			LOGE("addScaleKeyFrame,the Node is already has scale keyframe info");
		}
	}
	void addRotateKeyFrame(const std::string& nodeName,
		std::unique_ptr<std::vector<KeyFrameQuatTime>>& info) {
		if (!mNodesRotateKeyFrameInfo.emplace(nodeName, std::move(info)).second) {
			LOGE("addRotateKeyFrame,the Node is already has rotate keyframe info");
		}
	}

	void addNodeName(const std::string& name) {
		if (!mNodesNameSet.emplace(name).second) {
			LOGE("duplicate node name in NodeAnimation");
		}
	}

	void animate(Mesh* pMesh);

	bool findBone(const std::string& name);
private:
	std::string mName;
	std::shared_ptr<Node> mpRootNode;
	int64_t mDuration{ 0 };//in ms
	//the animation key frame info
	//nodeName-keyFrameInfo
	std::unordered_map < std::string, std::unique_ptr< std::vector<KeyFrameVec3Time> > > mNodesPosKeyFrameInfo;
	std::unordered_map < std::string, std::unique_ptr< std::vector<KeyFrameVec3Time> > > mNodesScaleKeyFrameInfo;
	std::unordered_map < std::string, std::unique_ptr< std::vector<KeyFrameQuatTime> > > mNodesRotateKeyFrameInfo;
	std::unordered_set<std::string> mNodesNameSet;
};