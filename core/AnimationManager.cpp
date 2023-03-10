#include "Animation.h"
#include "AnimationManager.h"
#include <algorithm>
#include "animation/SkeletonAnimation.h"
#include "Node.h"
AnimationManager& AnimationManager::getInstance() {
	static AnimationManager gManager;
	return gManager;
}

void AnimationManager::startAnimation(int i) {
	for (auto it = mAnimationMap.begin(); it != mAnimationMap.end();) {
		if (--i < 0) {
			it->second->start();
			mActiveAnimations.try_emplace(it->first, it->second);
			break;
		}
		else {
			++it;
		}
	}
}

void AnimationManager::startAnimation(const std::string& name) {
	auto it = mAnimationMap.find(name);
	if (it != std::end(mAnimationMap)) {
		if (it->second && !it->second->isStarted()) {
			it->second->start();
			mActiveAnimations.try_emplace(it->first,it->second);
		}
	}
}
void AnimationManager::stopAnimation(const std::string& name) {
	auto it = mActiveAnimations.find(name);
	if (it != mActiveAnimations.end()) {
		it->second->stop();
		mActiveAnimations.erase(it);
	}
}

void AnimationManager::updateActiveAnimation() {
	for (auto& pair : mActiveAnimations) {
		pair.second->animate();
	}
}

//void AnimationManager::addAffectedNode(const std::string& name, const std::shared_ptr<Node>& pNode) {
//	for (const auto& pair : mAnimationMap) {
//		auto pSkeletonAnimation = std::dynamic_pointer_cast<SkeletonAnimation>(pair.second);
//		if (pSkeletonAnimation) {
//			//pSkeletonAnimation->addAffectedNode(name, pNode);
//		}
//	}
//}