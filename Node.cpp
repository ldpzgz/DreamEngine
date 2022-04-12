#include "Node.h"
#include "NodeListener.h"
#include "Mesh.h"
#include "Log.h"
#include <glm/trigonometric.hpp>  //radians
#include <glm/ext/matrix_transform.hpp> // perspective, translate, rotate
#include <glm/gtc/type_ptr.hpp> // value_ptr
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

bool Node::addRenderable(const shared_ptr<Renderable>& temp) {
	unsigned int rid = sCurMeshId++;
	temp->setRid(rid);
	mRenderables.emplace(rid, temp);
	return true;
}


void Node::setMatrix(const glm::mat4& matrix) noexcept {
	mMat = matrix;
	updateChildWorldMatrix();
}


void Node::translate(float x, float y, float z) noexcept {
	glm::mat4 temp{ 1.0f };
	mMat = glm::translate(temp, glm::vec3(x, y, z)) * mMat;
	updateChildWorldMatrix();
}

void Node::rotate(float angle, const glm::vec3& vec) noexcept {
	glm::mat4 temp{ 1.0f };
	mMat = glm::rotate(temp, angle, vec) * mMat;
	updateChildWorldMatrix();
}

void Node::scale(const glm::vec3& scaleVec) noexcept {
	glm::mat4 temp{ 1.0f };
	mMat = glm::scale(temp, scaleVec) * mMat;
	updateChildWorldMatrix();
}


void Node::lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up) noexcept {
	mMat = glm::lookAt(eyepos, center, up);
	updateChildWorldMatrix();
}

void Node::addListener(const shared_ptr<NodeListener>& lis) {
	mListeners.emplace_back(lis);
}

void Node::updateChildWorldMatrix() noexcept {
	if (!mChildren.empty()) {
		auto myWorldMat = mParentWorldMat*mMat;
		for (auto& child : mChildren) {
			child.second->setParentWorldMatrix(myWorldMat);
			child.second->updateChildWorldMatrix();
		}
		for (auto& listener : mListeners) {
			if (listener) {
				listener->update(myWorldMat);
			}
		}
	}
}