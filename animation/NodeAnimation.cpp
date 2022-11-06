#include "NodeAnimation.h"
#include "core/Node.h"
#include <glm/ext/quaternion_common.hpp>
#include <glm/ext/quaternion_transform.hpp>
#include <glm/gtx/quaternion.hpp>
void NodeAnimation::setPosKeyFrame(const std::string& nodeName, float* pTime, glm::vec3* pPos, int count, InterpolationType interType) {
	if (pTime!=nullptr && pPos!=nullptr && count>0) {
		mpPosKeyFrames = std::make_unique<std::vector<KeyFrameVec3Time>>();
		mPosInterpolationType = interType;
		for (int i = 0; i < count; ++i) {
			int64_t time = static_cast<double>(pTime[i]) * 1000;
			mpPosKeyFrames->emplace_back(pPos[i], time);
		}
	}
}

void NodeAnimation::setScaleKeyFrame(const std::string& nodeName, float* pTime, glm::vec3* pScale, int count, InterpolationType interType) {
	if (pTime != nullptr && pScale != nullptr && count > 0) {
		mpScaleKeyFrames = std::make_unique<std::vector<KeyFrameVec3Time>>();
		mScaleInterpolationType = interType;
		for (int i = 0; i < count; ++i) {
			int64_t time = static_cast<double>(pTime[i]) * 1000;
			mpScaleKeyFrames->emplace_back(pScale[i], time);
		}
	}
}

void NodeAnimation::setRotateKeyFrame(const std::string& nodeName, float* pTime, glm::quat* pRotate, int count, InterpolationType interType) {
	if (pTime != nullptr && pRotate != nullptr && count > 0) {
		mpRotateKeyFrames = std::make_unique<std::vector<KeyFrameQuatTime>>();
		mRotateInterpolationType = interType;
		if (count > 0 && pTime[0]>0) {
			mpRotateKeyFrames->emplace_back(glm::quat(1.0f,0.0f,0.0f,0.0f), 0);
		}
		for (int i = 0; i < count; ++i) {
			int64_t time = static_cast<double>(pTime[i]) * 1000;
			mpRotateKeyFrames->emplace_back(pRotate[i], time);
		}
	}
}

void NodeAnimation::animate() {
	if (isStarted() && mpTargetNode) {
		int64_t curTime = mTimer.elapseFromStart() % mDuration;

		glm::mat4 posMatrix{ 1.0f };
		glm::mat4 scaleMatrix{ 1.0f };
		glm::mat4 rotateMatrix{ 1.0f };
		
		if (mpPosKeyFrames) {
			//evaluate the pos 
			if (mpPosKeyFrames->size() == 1) {
				posMatrix = glm::translate(posMatrix, (*mpPosKeyFrames)[0].vec);
			}
			else {
				auto nextTimeIt = std::lower_bound(mpPosKeyFrames->begin(), mpPosKeyFrames->end(), curTime,
					[](const KeyFrameVec3Time& key, int64_t time)->bool {
						return key.timeMs < time;
					});
				if (nextTimeIt != mpPosKeyFrames->end()) {
					auto preTimeIt = mpPosKeyFrames->begin();
					if (nextTimeIt == preTimeIt) {
						++nextTimeIt;
					}
					else {
						preTimeIt = nextTimeIt - 1;
					}
					float blendFactor = static_cast<float>(curTime - preTimeIt->timeMs) / static_cast<float>(nextTimeIt->timeMs - preTimeIt->timeMs);
					if (mPosInterpolationType == InterpolationType::Step) {
						blendFactor = glm::step(0.5f,blendFactor);
					}
					glm::vec3 curPos = glm::mix(preTimeIt->vec, nextTimeIt->vec, blendFactor);
					posMatrix = glm::translate(posMatrix, curPos);
				}
			}
		}

		
		if (mpScaleKeyFrames) {
			//evaluate the scale 
			if (mpScaleKeyFrames->size() == 1) {
				scaleMatrix = glm::scale(scaleMatrix, (*mpScaleKeyFrames)[0].vec);
			}
			else {
				auto nextTimeIt = std::lower_bound(mpScaleKeyFrames->begin(), 
					mpScaleKeyFrames->end(), curTime,
					[](const KeyFrameVec3Time& key, int64_t time)->bool {
						return key.timeMs < time;
					});
				if (nextTimeIt != mpScaleKeyFrames->end()) {
					auto preTimeIt = mpScaleKeyFrames->begin();
					if (nextTimeIt == preTimeIt) {
						++nextTimeIt;
					}
					else {
						preTimeIt = nextTimeIt - 1;
					}
					float blendFactor = static_cast<float>(curTime - preTimeIt->timeMs) / static_cast<float>(nextTimeIt->timeMs - preTimeIt->timeMs);
					if (mScaleInterpolationType == InterpolationType::Step) {
						blendFactor = glm::step(0.5f, blendFactor);
					}
					glm::vec3 curScale = glm::mix(preTimeIt->vec, nextTimeIt->vec, blendFactor);
					scaleMatrix = glm::scale(scaleMatrix, curScale);
				}
			}
		}

		if (mpRotateKeyFrames) {
			//evaluate the rotate
			if (mpRotateKeyFrames->size() == 1) {
				rotateMatrix = glm::toMat4((*mpRotateKeyFrames)[0].rotate);
			}
			else {
				auto nextTimeIt = std::lower_bound(mpRotateKeyFrames->begin(), 
					mpRotateKeyFrames->end(), curTime,
					[](const KeyFrameQuatTime& key, int64_t time)->bool {
						return key.timeMs < time;
					});
				if (nextTimeIt != mpRotateKeyFrames->end()) {
					auto preTimeIt = mpRotateKeyFrames->begin();
					if (nextTimeIt == preTimeIt) {
						++nextTimeIt;
					}
					else {
						preTimeIt = nextTimeIt - 1;
					}
					float blendFactor = static_cast<float>(curTime - preTimeIt->timeMs) / static_cast<float>(nextTimeIt->timeMs - preTimeIt->timeMs);
					glm::quat curRotate(1.0f,0.0f,0.0f,0.0f);
					if (mRotateInterpolationType == InterpolationType::Step) {
						blendFactor = glm::step(0.5f, blendFactor);
						curRotate = glm::mix(preTimeIt->rotate, nextTimeIt->rotate, blendFactor);
					}
					else if (mRotateInterpolationType == InterpolationType::Linear) {
						curRotate = glm::mix(preTimeIt->rotate, nextTimeIt->rotate, blendFactor);
					}
					curRotate = glm::slerp(preTimeIt->rotate, nextTimeIt->rotate, blendFactor);
					rotateMatrix = glm::toMat4(curRotate);
				}
			}
		}
		auto myLocalMat = posMatrix * rotateMatrix * scaleMatrix;
		mpTargetNode->setAny(NodeAnyIndex::NodeAnimationMatrix, myLocalMat);
		//mpTargetNode->setLocalMatrix(myLocalMat, false);
	}
}