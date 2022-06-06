#pragma once

#include <glm/vec3.hpp>           // vec3
#include <glm/vec4.hpp>           // vec3
#include <glm/mat4x4.hpp>         // mat4
#include <vector>
#include <unordered_map>
#include <memory>

constexpr int MAX_BONE_COUNT = 128;
class Node;
class Animation;
class Skeleton {
public:
	Skeleton() = default;
	~Skeleton() = default;
	explicit Skeleton(const std::string& name) {
		mName = name;
	}
	
	const std::string& getName() {
		return mName;
	}

	void setName(const std::string& name) {
		mName = name;
	}

	auto& getBoneName2Index() {
		return mBoneName2Index;
	}
	auto& getOffsetMatrix() {
		return mBonesOffsetMatrix;
	}

	auto& getFinalMatrix() {
		return mBonesFinalMatrix;
	}

	void setOffsetMatrix(glm::mat4* pMat, int count);

	std::shared_ptr<Node>& getRootNode() {
		return mpRootNode;
	}
	void setRootNode(const std::shared_ptr<Node>& pNode) {
		mpRootNode = pNode;
	}

	void addAnimation(const std::shared_ptr<Animation>& pa);

	void updateToUbo();

	void setUpdateFinalMatrix(bool b) {
		mbUpdatedFinalMatrix = b;
	}
private:
	std::string mName;

	std::vector<glm::mat4> mBonesOffsetMatrix;//this should be shared
	std::unordered_map<std::string, int> mBoneName2Index;//this should be shared too
	
	bool mbUpdatedFinalMatrix{ true };
	std::vector<glm::mat4> mBonesFinalMatrix;//when clone SkeletonAnimation,this should be cloned too
	std::shared_ptr<Node> mpRootNode;//when clone SkeletonAnimation,node tree should be cloned too

	std::unordered_map<std::string, std::shared_ptr<Animation>> mSkeletonAnimations;
};