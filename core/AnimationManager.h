#pragma once

#include "Log.h"
#include <unordered_map>
#include <list>
#include <memory>
class Animation;
class Node;
class AnimationManager {
public:
	static AnimationManager& getInstance();
	void addAnimation(const std::string& name, const std::shared_ptr<Animation>& pAnim) {
		if (!mAnimationMap.try_emplace(name, pAnim).second) {
			LOGE("duplicate animation name %s",name.c_str());
		}
	}
	void startAnimation(int i);
	void startAnimation(const std::string& name);
	void stopAnimation(const std::string& name);
	void updateActiveAnimation();
	//void addAffectedNode(const std::string& name,const std::shared_ptr<Node>& pNode);
private:
	std::unordered_map<std::string, std::shared_ptr<Animation>> mAnimationMap;
	std::unordered_map<std::string, std::shared_ptr<Animation>> mActiveAnimations;
};
