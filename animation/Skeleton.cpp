#include "Skeleton.h"
#include "SkeletonAnimation.h"
#include "core/Ubo.h"

void Skeleton::addAnimation(const std::shared_ptr<Animation>& pa) {
	if (pa) {
		mSkeletonAnimations.try_emplace(pa->getName(), pa);
	}
}

void Skeleton::updateToUbo() {
	if (mbUpdatedFinalMatrix) {
		Ubo::getInstance().update("Bones", mBonesFinalMatrix.data(), mBonesFinalMatrix.size() * sizeof(glm::mat4));
		mbUpdatedFinalMatrix = false;
	}
}

void Skeleton::setOffsetMatrix(glm::mat4* pMat, int count) {
	mBonesOffsetMatrix.assign(pMat, pMat + count);
	mBonesFinalMatrix.assign(MAX_BONE_COUNT, glm::mat4(1.0f));
}