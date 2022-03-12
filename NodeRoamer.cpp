#include "Node.h"
#include "NodeRoamer.h"
#include <glm/ext/matrix_transform.hpp> // perspective, translate, rotate
/*
* 操纵节点漫游
*/

void NodeRoamer::setTarget(std::shared_ptr<Node>& pNode,std::shared_ptr<Node>& pView) {
	mpNode = pNode;
	mpView = pView;
	mStartRotateX = 0;
	mStartRotateY = 0;
	mIsStartRotate = false;
}
void NodeRoamer::startRotate(int x, int y) {
	if (mpNode) {
		mStartRotateX = x;
		mStartRotateY = y;
		mIsStartRotate = true;
	}
}
void NodeRoamer::rotate(int x, int y) {
	if (mpNode) {
		if (!mIsStartRotate)
			return;
		float a = float(x - mStartRotateX);
		float b = float(y - mStartRotateY);
		float len = sqrtf(a * a + b * b);
		glm::mat4 tempLr{ 1.0f };
		glm::mat4 tempUd{ 1.0f };
		if (fabs(a) > 0.99f) {
			tempLr = glm::rotate(tempLr, a / 100.0f, glm::vec3(0.0f, 1.0f, 0.0f));
		}
		if (fabs(b)> 0.99f) {
			auto& viewMat = mpView->getMatrix();
			//view矩阵左上角的旋转矩阵一般都是正交矩阵
			tempUd = glm::rotate(tempUd, b / 100.0f, glm::vec3(viewMat[0][0], viewMat[1][0], viewMat[2][0]));
		}
		const auto& myMat = mpNode->getMatrix();
		
		//模仿blender查看物体的方式
		mpNode->setMatrix(tempUd *myMat * tempLr);

		mStartRotateX = x;
		mStartRotateY = y;

		/*if (len > 0.99f) {
			glm::mat4 tempM{ 1.0f };
			tempM = glm::rotate(tempM, len / 100.0f, glm::vec3(b, a, 0.0f));
			auto& oMat = mpNode->getMatrix();
			oMat = tempM * oMat;
			mpNode->updateChildWorldMatrix();
			mStartRotateX = x;
			mStartRotateY = y;
		}*/
	}
}
void NodeRoamer::endRotate(int x, int y) {
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
		auto& oMat = mpNode->getMatrix();
		oMat = tempM * oMat;
	}
}