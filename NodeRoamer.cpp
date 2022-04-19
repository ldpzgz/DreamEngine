#include "Camera.h"
#include "Node.h"
#include "NodeRoamer.h"
#include "Mesh.h"
#include "aabb.h"
#include <glm/ext/matrix_transform.hpp> // perspective, translate, rotate
/*
* 操纵节点漫游
*/

void NodeRoamer::setTarget(std::shared_ptr<Node>& pNode,std::shared_ptr<Camera>& pView) {
	mpNode = pNode;
	mpView = pView;
	mStartRotateX = 0;
	mStartRotateY = 0;
	mIsStartRotate = false;
	AABB aabb;
	if (mpNode) {
		mpNode->visitNode([&aabb](Node* pNode) {
			if (pNode) {
				for (const auto& pMesh : pNode->getRenderables()) {
					auto pM = std::dynamic_pointer_cast<Mesh>(pMesh.second);
					if (pM) {
						auto& pab = pM->getAabb();
						if (pab) {
							aabb += *pab;
						}
					}
				}
			}
			});
	}
	mLengthBase = glm::length(aabb.length());
	if (mLengthBase < 0.0001f) {
		mLengthBase = 1.0f;
	}
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
			auto& viewMat = mpView->getViewMatrix();
			auto& parentMat = mpNode->getParentWorldMat();
			auto tempMat = viewMat * parentMat;
			//view矩阵左上角的旋转矩阵一般都是正交矩阵
			tempUd = glm::rotate(tempUd, b / 100.0f, glm::vec3(tempMat[0][0], tempMat[1][0], tempMat[2][0]));
		}
		const auto& myMat = mpNode->getLocalMatrix();
		const auto& myParentMat = mpNode->getParentWorldMat();
		
		//模仿blender查看物体的方式
		mpNode->setLocalMatrix(tempUd * myMat * tempLr);

		mStartRotateX = x;
		mStartRotateY = y;
	}
}
void NodeRoamer::endRotate(int x, int y) {
	mIsStartRotate = false;
}
void NodeRoamer::move(bool front) {
	if (mpView) {
		glm::mat4 tempM{ 1.0f };
		float delta = mLengthBase / 10.0f;
		if (front) {
			tempM = glm::translate(tempM, glm::vec3(0.0f, 0.0f, -delta));
		}
		else {
			tempM = glm::translate(tempM, glm::vec3(0.0f, 0.0f, delta));
		}
		auto& oMat = mpView->getViewMatrix();
		oMat = tempM * oMat;
	}
}