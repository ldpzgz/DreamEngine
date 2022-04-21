#include "../Node.h"
#include "../mesh.h"
#include "NodeAnimation.h"
#include <algorithm>
#include<glm/ext/quaternion_common.hpp>
#include<glm/ext/quaternion_transform.hpp>
#include <glm/gtx/quaternion.hpp>

void NodeAnimation::updateAffectedMesh() {
	auto rootMat = mpRootNode->getWorldMatrix();
	auto rootMatInv = glm::inverse(rootMat);
	for (auto& pairNameNode : mNameNodeMap) {
		for (auto& pMesh : mAffectedMeshes) {
			auto& nameIdMap = pMesh->getBoneNameIndex();
			auto nameId = nameIdMap.find(pairNameNode.first);
			if (nameId != nameIdMap.end()) {
				auto& offsetMatrixs = pMesh->getBonesOffsetMatrix();
				auto& finalMatrix = pMesh->getBonesFinalMatrix();
				auto worldMat = pairNameNode.second->getWorldMatrix();
				finalMatrix[nameId->second] = rootMatInv * worldMat * offsetMatrixs[nameId->second];
			}
		}
	}
}

void NodeAnimation::animate() {
	if (isStarted() && mpRootNode) {
		int completeCount = 0;
		bool visitOver = false;
		int64_t curTime = mTimer.elapseFromStart()% mDuration;
		mpRootNode->visitNode([this,&completeCount,&curTime](Node* pNode)->bool {
			const std::string& nodeName = pNode->getName();
			bool bVisit = mNameNodeMap.find(nodeName) != std::end(mNameNodeMap);
			if (bVisit && !nodeName.empty()) {
				glm::mat4 posMatrix{1.0f};
				glm::mat4 scaleMatrix{ 1.0f };
				glm::mat4 rotateMatrix{ 1.0f };
				auto posKeyIt = mNodesPosKeyFrameInfo.find(nodeName);
				if (posKeyIt != mNodesPosKeyFrameInfo.end()) {
					//evaluate the pos 
					//find two keys which contains the curtime
					auto& posArray = *posKeyIt->second;
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

				auto scaleKeyIt = mNodesScaleKeyFrameInfo.find(nodeName);
				if (scaleKeyIt != mNodesScaleKeyFrameInfo.end()) {
					//evaluate the scale 
					auto& scaleArray = *scaleKeyIt->second;
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

				auto rotateKeyIt = mNodesRotateKeyFrameInfo.find(nodeName);
				if (rotateKeyIt != mNodesRotateKeyFrameInfo.end()) {
					//evaluate the rotate
					auto& rotateArray = *rotateKeyIt->second;
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
				auto myWorldMat = pNode->getParentWorldMat() * myLocalMat;
				pNode->setLocalMatrix(myLocalMat);
				//update the parentWorldMatrix of pNode's all direct children
				auto& children = pNode->getChildren();
				for(auto& pChild: children){
					pChild->setParentWorldMatrix(myWorldMat,false);
				}

				++completeCount;
				if (completeCount == mNameNodeMap.size()) {
					return true;
				}
			}
			else {
				auto myWorldMat = pNode->getWorldMatrix();
				//update the parentWorldMatrix of pNode's all direct children
				auto& children = pNode->getChildren();
				for (auto& pChild : children){
					pChild->setParentWorldMatrix(myWorldMat, false);
				}
			}
			return false;
		}, visitOver);

		updateAffectedMesh();
	}
}

bool NodeAnimation::findBone(const std::string& name) {
	return mNameNodeMap.find(name) != mNameNodeMap.end();
}
