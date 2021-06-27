#include "Node.h"
#include "Log.h"
Node::Node():
	mCurChileId(0),
	mCurMeshId(0),
	mMat(1.0f),
	mId(0)
{

}

Node::~Node() {

}

shared_ptr<Node> Node::newAChild() {
	auto child = make_shared<Node>();
	addChild(child);
	return child;
}

bool Node::hasParent() {
	return !mpParent.expired();
}

void Node::setParent(shared_ptr<Node> parent) {
	mpParent = parent;
}

bool Node::removeChild(unsigned int childId) {
	return mChildren.erase(childId) == 1 ? true : false;
}

bool Node::addChild(shared_ptr<Node>& child) {
	if (child->hasParent()) {
		LOGD("node cannot add a child which has parent");
		return false;
	}
	unsigned int id = mCurChileId++;
	child->setId(id);
	auto thisptr = shared_from_this();
	child->setParent(thisptr);
	return mChildren.insert(make_pair(id, child)).second;
}

bool Node::addMesh(shared_ptr<Mesh>& temp) {
	return mMeshes.insert(make_pair(mCurMeshId++,temp)).second;
}

void Node::setMatrix(glm::mat4& matrix) {
	mMat = matrix;
}