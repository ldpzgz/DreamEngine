#include "Node.h"
#include "NodeListener.h"
#include "Mesh.h"
#include "Log.h"
#include <glm/trigonometric.hpp>  //radians
#include <glm/ext/matrix_transform.hpp> // perspective, translate, rotate
#include <glm/gtc/type_ptr.hpp> // value_ptr

atomic_uint Node::sCurMeshId = 0;

Node::Node() = default;

Node::~Node() = default;

shared_ptr<Node> Node::newAChild() {
	auto child = make_shared<Node>();
	addChild(child);
	return child;
}

bool Node::addChild(shared_ptr<Node>& child) {
	if (child) {
		child->setParent(shared_from_this());
		child->setParentWorldMatrix(getWorldMatrix());
	}
	if (!mpFirstChild) {
		mpFirstChild = child;
		mpLastChild = child;
	}
	else {
		mpLastChild->mpNextSibling = child;
		child->mpPreSibling = mpLastChild;
		mpLastChild = child;
	}
	return true;
}

bool Node::removeChild(shared_ptr<Node>& child) noexcept{
	auto p = mpFirstChild;
	while (p && p != child) {
		p = p->mpNextSibling;
	}
	if (p) {
		auto& pre = p->mpPreSibling.lock();
		auto& next = p->mpNextSibling;
		if (pre) {
			pre->mpNextSibling = next;
		}
		else {
			//remove first child;
			mpFirstChild = next;
		}

		if (next) {
			next->mpPreSibling = pre;
		}
		else {
			//remove last child;
			mpLastChild = pre;
		}
		child->mpNextSibling.reset();
		child->mpPreSibling.reset();
		child.reset();
		return true;
	}
	return false;
}

bool Node::addRenderable(const shared_ptr<Renderable>& temp) {
	unsigned int rid = sCurMeshId++;
	temp->setRid(rid);
	mRenderables.emplace(rid, temp);
	return true;
}


void Node::setLocalMatrix(const glm::mat4& matrix) noexcept {
	mLocalMat = matrix;
	if (mpFirstChild) {
		mpFirstChild->updateMatrixHierarchy(mParentWorldMat * mLocalMat);
	}
}

void Node::setLocalMatrixOnlyChild(const glm::mat4& matrix) noexcept {
	mLocalMat = matrix;
	if (mpFirstChild) {
		mpFirstChild->updateMatrixMeAndSibling(mParentWorldMat * mLocalMat);
	}
}

void Node::translate(float x, float y, float z) noexcept {
	glm::mat4 temp{ 1.0f };
	mLocalMat = glm::translate(temp, glm::vec3(x, y, z)) * mLocalMat;
	if (mpFirstChild) {
		mpFirstChild->updateMatrixHierarchy(mParentWorldMat * mLocalMat);
	}
}

void Node::rotate(float angle, const glm::vec3& vec) noexcept {
	glm::mat4 temp{ 1.0f };
	mLocalMat = glm::rotate(temp, angle, vec) * mLocalMat;
	if (mpFirstChild) {
		mpFirstChild->updateMatrixHierarchy(mParentWorldMat * mLocalMat);
	}
}

void Node::scale(const glm::vec3& scaleVec) noexcept {
	glm::mat4 temp{ 1.0f };
	mLocalMat = glm::scale(temp, scaleVec) * mLocalMat;
	if (mpFirstChild) {
		mpFirstChild->updateMatrixHierarchy(mParentWorldMat * mLocalMat);
	}
}

void Node::visitNode(const std::function<void(Node*)>& func) {
	if (func) {
		func(this);
	}
	if (mpNextSibling) {
		mpNextSibling->visitNode(func);
	}
	if (mpFirstChild) {
		mpFirstChild->visitNode(func);
	}
}
//visit node£¬can be break
void Node::visitNode(const std::function<bool(Node*)>& func, bool& isOver) {
	if (func) {
		isOver = func(this);
	}
	if (mpNextSibling && !isOver) {
		mpNextSibling->visitNode(func, isOver);
	}
	if (mpFirstChild && !isOver) {
		mpFirstChild->visitNode(func, isOver);
	}
}

void Node::lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up) noexcept {
	mLocalMat = glm::lookAt(eyepos, center, up);
	if (mpFirstChild) {
		mpFirstChild->updateMatrixHierarchy(mParentWorldMat*mLocalMat);
	}
}

void Node::addListener(const shared_ptr<NodeListener>& lis) {
	mListeners.emplace_back(lis);
}
/*
* for situation: only parent is moveing,and all the descendant node of parent is static
*/
void Node::updateMatrixHierarchy(glm::mat4& parentMat) noexcept {
	setParentWorldMatrix(parentMat);
	//update sibling node
	if (mpNextSibling) {
		mpNextSibling->updateMatrixHierarchy(parentMat);
	}
	if (mpFirstChild) {
		//update all my descendant
		mpFirstChild->updateMatrixHierarchy(mParentWorldMat * mLocalMat);
	}
	for (auto& listener : mListeners) {
		if (listener) {
			listener->update(mParentWorldMat * mLocalMat);
		}
	}
}

/*
* for situation: parent is moving, and the descendant of parent node will be moving,
* thus It is not necessary to update all descendant
*/
void Node::updateMatrixMeAndSibling(glm::mat4& parentMat) noexcept {
	setParentWorldMatrix(parentMat);
	//update sibling node
	if (mpNextSibling) {
		mpNextSibling->updateMatrixMeAndSibling(parentMat);
	}
	for (auto& listener : mListeners) {
		if (listener) {
			listener->update(mParentWorldMat * mLocalMat);
		}
	}
}