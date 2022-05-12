#include "../Node.h"
#include "../mesh.h"
#include "NodeAnimation.h"
#include "Skeleton.h"
#include <algorithm>
#include<glm/ext/quaternion_common.hpp>
#include<glm/ext/quaternion_transform.hpp>
#include <glm/gtx/quaternion.hpp>

NodeAnimation::NodeAnimation(const std::string& name) :
	Animation(name),
	mpNodesPosKeyFrameInfo(std::make_shared< std::unordered_map< std::string, std::vector<KeyFrameVec3Time>>>()),
	mpNodesScaleKeyFrameInfo(std::make_shared<std::unordered_map< std::string, std::vector<KeyFrameVec3Time>>>()),
	mpNodesRotateKeyFrameInfo(std::make_shared<std::unordered_map< std::string, std::vector<KeyFrameQuatTime>>>())
{

}

NodeAnimation::NodeAnimation():
	mpNodesPosKeyFrameInfo(std::make_shared< std::unordered_map< std::string, std::vector<KeyFrameVec3Time>>>()),
	mpNodesScaleKeyFrameInfo(std::make_shared<std::unordered_map< std::string, std::vector<KeyFrameVec3Time>>>()),
	mpNodesRotateKeyFrameInfo(std::make_shared<std::unordered_map< std::string, std::vector<KeyFrameQuatTime>>>())
{

}

void NodeAnimation::addPosKeyFrame(const std::string& nodeName, float* pTime, glm::vec3* pPos, int count) {
	std::vector< KeyFrameVec3Time> vec;
	for (int i = 0; i < count; ++i) {
		int64_t time = pTime[i] * 1000;
		vec.emplace_back( pPos[i],time);
	}
	if (!mpNodesPosKeyFrameInfo->try_emplace(nodeName, std::move(vec)).second) {
		LOGE("addPosKeyFrame,the Node is already has pos keyframe info");
	}
}

void NodeAnimation::addScaleKeyFrame(const std::string& nodeName, float* pTime, glm::vec3* pScale,int count) {
	std::vector< KeyFrameVec3Time> vec;
	for (int i = 0; i < count; ++i) {
		int64_t time = pTime[i] * 1000;
		vec.emplace_back( pScale[i],time);
	}
	if (!mpNodesScaleKeyFrameInfo->try_emplace(nodeName, std::move(vec)).second) {
		LOGE("addPosKeyFrame,the Node is already has pos keyframe info");
	}
}

void NodeAnimation::addRotateKeyFrame(const std::string& nodeName, float* pTime, glm::quat* pQuat, int count) {
	std::vector< KeyFrameQuatTime> vec;
	for (int i = 0; i < count; ++i) {
		int64_t time = pTime[i] * 1000;
		vec.emplace_back( pQuat[i],time);
	}
	if (!mpNodesRotateKeyFrameInfo->try_emplace(nodeName, std::move(vec)).second) {
		LOGE("addPosKeyFrame,the Node is already has pos keyframe info");
	}
}

void NodeAnimation::animate() {
	if (isStarted() && !mpSkeleton.expired()) {
		//int completeCount = 0;
		auto pSkeleton = mpSkeleton.lock();
		if (!pSkeleton) {
			return;
		}
		bool visitOver = false;
		int64_t curTime = mTimer.elapseFromStart()% mDuration;
		auto& pRootNode = pSkeleton->getRootNode();
		if (!pRootNode) {
			LOGD("the rootNode of skeleton is null in animation %s",mName.c_str());
			return;
		}

		auto& offsetMatrixes = pSkeleton->getOffsetMatrix();
		auto& finalMatrixes = pSkeleton->getFinalMatrix();
		auto& name2Index = pSkeleton->getBoneName2Index();
		glm::mat4 invertRootMatrix(1.0f);
		if (name2Index.find(pRootNode->getName()) == name2Index.end()) {
			invertRootMatrix = glm::inverse(pRootNode->getWorldMatrix());
		}

		pRootNode->visitNode([this,&pSkeleton ,&curTime,&invertRootMatrix,&name2Index,&offsetMatrixes,&finalMatrixes](Node* pNode) {//&completeCount,
			const std::string& nodeName = pNode->getName();
			if (!nodeName.empty()) {
				glm::mat4 posMatrix{1.0f};
				glm::mat4 scaleMatrix{ 1.0f };
				glm::mat4 rotateMatrix{ 1.0f };
				auto posKeyIt = mpNodesPosKeyFrameInfo->find(nodeName);
				if (posKeyIt != mpNodesPosKeyFrameInfo->end()) {
					//evaluate the pos 
					//find two keys which contains the curtime
					auto& posArray = posKeyIt->second;
					if (posArray.size() == 1) {
						posMatrix = glm::translate(posMatrix, posArray[0].vec);
					}
					else {
						auto nextTimeIt = std::lower_bound(posArray.begin(), posArray.end(), curTime,
							[](const KeyFrameVec3Time& key, int64_t time)->bool {
								return key.timeMs < time;
							});
						if (nextTimeIt != posArray.end()) {
							auto preTimeIt = posArray.begin();
							if (nextTimeIt == preTimeIt) {
								++nextTimeIt;
							}
							else {
								preTimeIt = nextTimeIt - 1;
							}
							float blendFactor = static_cast<float>(curTime - preTimeIt->timeMs) / static_cast<float>(nextTimeIt->timeMs - preTimeIt->timeMs);
							glm::vec3 curPos = glm::mix(preTimeIt->vec, nextTimeIt->vec, blendFactor);
							posMatrix = glm::translate(posMatrix, curPos);
						}
					}
				}

				auto scaleKeyIt = mpNodesScaleKeyFrameInfo->find(nodeName);
				if (scaleKeyIt != mpNodesScaleKeyFrameInfo->end()) {
					//evaluate the scale 
					auto& scaleArray = scaleKeyIt->second;
					if (scaleArray.size() == 1) {
						scaleMatrix = glm::scale(scaleMatrix, scaleArray[0].vec);
					}
					else {
						auto nextTimeIt = std::lower_bound(scaleArray.begin(), scaleArray.end(), curTime,
							[](const KeyFrameVec3Time& key, int64_t time)->bool {
								return key.timeMs < time;
							});
						if (nextTimeIt != scaleArray.end()) {
							auto preTimeIt = scaleArray.begin();
							if (nextTimeIt == preTimeIt) {
								++nextTimeIt;
							}
							else {
								preTimeIt = nextTimeIt - 1;
							}
							float blendFactor = static_cast<float>(curTime - preTimeIt->timeMs) / static_cast<float>(nextTimeIt->timeMs - preTimeIt->timeMs);
							glm::vec3 curScale = glm::mix(preTimeIt->vec, nextTimeIt->vec, blendFactor);
							scaleMatrix = glm::scale(scaleMatrix, curScale);
						}
					}
				}

				auto rotateKeyIt = mpNodesRotateKeyFrameInfo->find(nodeName);
				if (rotateKeyIt != mpNodesRotateKeyFrameInfo->end()) {
					//evaluate the rotate
					auto& rotateArray = rotateKeyIt->second;
					if (rotateArray.size() == 1) {
						rotateMatrix = glm::toMat4(rotateArray[0].rotate);
					}
					else {
						auto nextTimeIt = std::lower_bound(rotateArray.begin(), rotateArray.end(), curTime,
							[](const KeyFrameQuatTime& key, int64_t time)->bool {
								return key.timeMs < time;
							});
						if (nextTimeIt != rotateArray.end()) {
							auto preTimeIt = rotateArray.begin();
							if (nextTimeIt == preTimeIt) {
								++nextTimeIt;
							}
							else {
								preTimeIt = nextTimeIt - 1;
							}
							float blendFactor = static_cast<float>(curTime - preTimeIt->timeMs) / static_cast<float>(nextTimeIt->timeMs - preTimeIt->timeMs);
							glm::quat curRotate = glm::slerp(preTimeIt->rotate, nextTimeIt->rotate, blendFactor);
							rotateMatrix = glm::toMat4(curRotate);
						}
					}
				}
				auto myLocalMat = posMatrix * rotateMatrix * scaleMatrix;
				pNode->setLocalMatrix(myLocalMat,false);

				auto it = name2Index.find(nodeName);
				if (it != name2Index.end()) {
					int index = it->second;
					auto worldMat = pNode->getWorldMatrix();
					finalMatrixes[index] = invertRootMatrix * worldMat * offsetMatrixes[index];
					pSkeleton->setUpdateFinalMatrix(true);
				}
			}
			else {
				LOGD("a node has no name in NodeAnimation");
				pNode->updateDirectChild(true);
			}
		});

		//updateAffectedMesh();
	}
}
