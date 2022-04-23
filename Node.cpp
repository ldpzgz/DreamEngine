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

void Node::addChild(shared_ptr<Node>& child) {
	if (child) {
		child->setParent(shared_from_this());
		child->setParentWorldMatrix(getWorldMatrix());
		mChildren.emplace_back(child);
	}
	/*if (!mpFirstChild) {
		mpFirstChild = child;
		mpLastChild = child;
	}
	else {
		mpLastChild->mpNextSibling = child;
		child->mpPreSibling = mpLastChild;
		mpLastChild = child;
	}
	return true;*/
}

void Node::removeChild(shared_ptr<Node>& child) noexcept{
	mChildren.remove(child);
	//auto p = mpFirstChild;
	//while (p && p != child) {
	//	p = p->mpNextSibling;
	//}
	//if (p) {
	//	auto& pre = p->mpPreSibling.lock();
	//	auto& next = p->mpNextSibling;
	//	if (pre) {
	//		pre->mpNextSibling = next;
	//	}
	//	else {
	//		//remove first child;
	//		mpFirstChild = next;
	//	}

	//	if (next) {
	//		next->mpPreSibling = pre;
	//	}
	//	else {
	//		//remove last child;
	//		mpLastChild = pre;
	//	}
	//	child->mpNextSibling.reset();
	//	child->mpPreSibling.reset();
	//	child.reset();
	//	return true;
	//}
	//return false;
}

bool Node::addRenderable(const shared_ptr<Renderable>& temp) {
	unsigned int rid = sCurMeshId++;
	temp->setRid(rid);
	mRenderables.emplace(rid, temp);
	return true;
}

void Node::setParentWorldMatrix(const glm::mat4& matrix, bool updateChild, bool notify) noexcept {
	mParentWorldMat = matrix;
	if (notify) {
		updateListener();
	}
	if (updateChild) {
		updateChildrenMatrix();
	}
}

void Node::setLocalMatrix(const glm::mat4& matrix,bool updateChild, bool notify) noexcept {
	mLocalMat = matrix;
	if (notify) {
		updateListener();
	}
	if (updateChild) {
		updateChildrenMatrix();
	}
}

void Node::translate(float x, float y, float z, bool updateChild, bool notify) noexcept {
	glm::mat4 temp{ 1.0f };
	mLocalMat = glm::translate(temp, glm::vec3(x, y, z)) * mLocalMat;
	if (notify) {
		updateListener();
	}
	if (updateChild) {
		updateChildrenMatrix();
	}
}

void Node::rotate(float angle, const glm::vec3& vec, bool updateChild, bool notify) noexcept {
	glm::mat4 temp{ 1.0f };
	mLocalMat = glm::rotate(temp, angle, vec) * mLocalMat;
	if (notify) {
		updateListener();
	}
	if (updateChild) {
		updateChildrenMatrix();
	}
}

void Node::scale(const glm::vec3& scaleVec, bool updateChild, bool notify) noexcept {
	glm::mat4 temp{ 1.0f };
	mLocalMat = glm::scale(temp, scaleVec) * mLocalMat;
	if (notify) {
		updateListener();
	}
	if (updateChild) {
		updateChildrenMatrix();
	}
}

void Node::visitNode(const std::function<void(Node*)>& func) {
	if (func) {
		func(this);
	}
	for (auto& pChild : mChildren) {
		if (pChild) {
			pChild->visitNode(func);
		}
	}
}
//visit node£¬can be break
void Node::visitNode(const std::function<bool(Node*)>& func, bool& isOver) {
	if (func) {
		isOver = func(this);
	}

	for (auto& pChild : mChildren) {
		if (isOver)
			return;
		if (pChild) {
			pChild->visitNode(func,isOver);
		}
	}
}

void Node::visitNode(const std::function<bool(Node*, bool& visitChild)>& func, bool& isOver) {
	bool visitChild = true;
	if (func) {
		isOver = func(this, visitChild);
	}
	if (!visitChild) {
		return;
	}
	for (auto& pChild : mChildren) {
		if (isOver)
			return;
		if (pChild) {
			pChild->visitNode(func, isOver);
		}
	}
}

void Node::lookAt(const glm::vec3& eyepos, const glm::vec3& center, 
	const glm::vec3& up, bool updateChild, bool notify) noexcept {
	mLocalMat = glm::lookAt(eyepos, center, up);
	if (notify) {
		updateListener();
	}
	if (updateChild) {
		updateChildrenMatrix();
	}
}

void Node::addListener(const shared_ptr<NodeListener>& lis) {
	mListeners.emplace_back(lis);
}

void Node::updateChildrenMatrix(bool notify) noexcept {
	if (!mChildren.empty()) {
		auto myWorldMat = mParentWorldMat * mLocalMat;
		for (auto& pNode : mChildren) {
			pNode ? pNode->setParentWorldMatrix(myWorldMat,true,notify) : 0;
		}
	}
}

void Node::updateListener() {
	if (!mListeners.empty()) {
		auto myWorldMat = mParentWorldMat * mLocalMat;
		for (auto& listener : mListeners) {
			if (listener) {
				listener->update(myWorldMat);
			}
		}
	}
}