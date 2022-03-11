#include "Node.h"
#include "Mesh.h"
#include "Log.h"

atomic_uint Node::sCurChildId=0;
atomic_uint Node::sCurMeshId=0;

Node::Node() :
	mId(0),
	mMat(1.0f),
	mParentWorldMat(1.0f)
{

}

Node::~Node() {

}

shared_ptr<Node> Node::newAChild() {
	auto child = make_shared<Node>();
	int id = sCurChildId++;
	if (mChildren.emplace(id, child).second) {
		child->setId(id);
		child->setParent(shared_from_this());
	}
	else {
		child.reset();
	}
	return child;
}

bool Node::addChild(shared_ptr<Node>& child) {
	//if (child->hasParent()) {
	//	//LOGE("node cannot add a child which has parent");
	//	return false;
	//}
	int id = sCurChildId++;
	if (mChildren.emplace(id, child).second) {
		child->setId(id);
		child->setParent(shared_from_this());
	}
	return true;
}

bool Node::removeChild(unsigned int childId) noexcept{
	return mChildren.erase(childId) == 1 ? true : false;
}

bool Node::addAttachment(const shared_ptr<Attachable>& temp) {
	mAttachments.emplace(sCurMeshId++, temp);
	return true;
}


void Node::setMatrix(const glm::mat4& matrix) noexcept {
	mMat = matrix;
	updateChildWorldMatrix();
}


void Node::translate(float x, float y, float z) noexcept {
	mMat = glm::translate(mMat, glm::vec3(x, y, z));
	updateChildWorldMatrix();
}

void Node::rotate(float angle, const glm::vec3& vec) noexcept {
	mMat = glm::rotate(mMat, angle, vec);
	updateChildWorldMatrix();
}

void Node::scale(const glm::vec3& scaleVec) noexcept {
	mMat = glm::scale(mMat, scaleVec);
	updateChildWorldMatrix();
}


void Node::lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up) noexcept {
	mMat = glm::lookAt(eyepos, center, up);
	updateChildWorldMatrix();
}

void Node::updateChildWorldMatrix() noexcept {
	if (!mChildren.empty()) {
		auto myWorldMat = mParentWorldMat * mMat;
		for (auto& child : mChildren) {
			child.second->setParentWorldMatrix(myWorldMat);
			child.second->updateChildWorldMatrix();
		}
	}
}