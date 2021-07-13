#include "Node.h"
#include "Log.h"
Node::Node():
	mCurChileId(0),
	mCurMeshId(0),
	mMat(1.0f),
	mParentWorldMat(1.0f),
	mId(0)
{

}

Node::~Node() {

}

shared_ptr<Node> Node::newAChild() {
	auto child = make_shared<Node>();
	unsigned int id = mCurChileId++;
	auto& it = mChildren.try_emplace(id, child);
	if (it.second) {
		auto& tempNode = it.first->second;
		tempNode->setId(id);
		auto thisptr = shared_from_this();
		tempNode->setParent(thisptr);
		return tempNode;
	}
	else {
		child.reset();
		return child;
	}
}

bool Node::hasParent() {
	return !mpParent.expired();
}

void Node::setParent(shared_ptr<Node>& parent) {
	mpParent = parent;
}

bool Node::removeChild(unsigned int childId) {
	return mChildren.erase(childId) == 1 ? true : false;
}

bool Node::addChild(shared_ptr<Node>& child) {
	if (child->hasParent()) {
		LOGE("node cannot add a child which has parent");
		return false;
	}
	unsigned int id = mCurChileId++;
	child->setId(id);
	auto thisptr = shared_from_this();
	child->setParent(thisptr);
	return mChildren.try_emplace(id, child).second;
}

bool Node::addMesh(shared_ptr<Mesh>& temp) {
	return mMeshes.try_emplace(mCurMeshId++,temp).second;
}

void Node::updateChildWorldMatrix() const noexcept {
	if (!mChildren.empty()) {
		auto myWorldMat = mParentWorldMat* mMat;
		std::for_each(mChildren.begin(), mChildren.end(), [&myWorldMat](const MapINode::value_type child) {
			child.second->setParentWorldMatrix(myWorldMat);
			child.second->updateChildWorldMatrix();
		});
	}
}

void Node::translate(float x, float y, float z) {
	mMat = glm::translate(mMat, glm::vec3(x, y, z));
	updateChildWorldMatrix();
}

void Node::lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up) {
	mMat = glm::lookAt(eyepos, center, up);
	updateChildWorldMatrix();
}

void Node::rotate(float angle, const glm::vec3& vec) {
	mMat = glm::rotate(mMat, angle, vec);
	updateChildWorldMatrix();
}

void Node::scale(const glm::vec3& scaleVec) {
	mMat = glm::scale(mMat, scaleVec);
	updateChildWorldMatrix();
}