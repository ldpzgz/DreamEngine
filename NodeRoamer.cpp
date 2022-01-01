#include "NodeRoamer.h"

/*
* ²Ù×Ý½ÚµãÂþÓÎ
*/

void NodeRoamer::setTarget(std::shared_ptr<Node<glm::mat4>>& pNode) {
	mpNode = pNode;
	mStartRotateX = 0;
	mStartRotateY = 0;
	mIsStartRotate = false;
	mOriginMat = glm::identity<glm::mat4>();
}
void NodeRoamer::startRotate(int x, int y) {
	if (mpNode) {
		mStartRotateX = x;
		mStartRotateY = y;
		mOriginMat = mpNode->getMatrix();
		mIsStartRotate = true;
	}
}
void NodeRoamer::rotate(int x, int y) {
	if (mpNode) {
		if (!mIsStartRotate)
			return;
		float a = float(x - mStartRotateX);
		float b = float(y - mStartRotateY);
		glm::mat4 tempM{ 1.0f };
		if (fabs(a) > fabs(b)) {
			tempM = glm::rotate(tempM, a / 100.0f, glm::vec3(0.0f, 1.0f, 0.0f));
			mpNode->setMatrix(tempM * mOriginMat);
		}
		else {
			tempM = glm::rotate(tempM, b / 100.0f, glm::vec3(1.0f, 0.0f, 0.0f));
			mpNode->setMatrix(tempM * mOriginMat);
		}
	}
}
void NodeRoamer::endRotate(int x, int y) {
	mOriginMat = glm::identity<glm::mat4>();
	mIsStartRotate = false;
}
void NodeRoamer::move(bool front) {
	if (mpNode) {
		glm::mat4 tempM{ 1.0f };
		if (front) {
			tempM = glm::translate(tempM, glm::vec3(0.0f, 0.0f, -5.0f));
		}
		else {
			tempM = glm::translate(tempM, glm::vec3(0.0f, 0.0f, 5.0f));
		}
		mpNode->setMatrix(tempM*mpNode->getMatrix());
	}
}