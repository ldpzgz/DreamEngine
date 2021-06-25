#include "Node.h"

Node::Node():
	mCurChileId(0)
{

}

shared_ptr<Node> Node::newAChild() {
	auto child = make_shared<Node>();
	mChildren[mCurChileId++] = child;
	return child;
}

bool Node::setParent(shared_ptr<Node> parent) {
	if (parent) {
		parent->addChild(shared_from_this());
		return true;
	}
	return false;
}

void Node::addChild(shared_ptr<Node> child) {
	mChildren[mCurChileId++] = child;
}