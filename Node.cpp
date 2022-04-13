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


void Node::setMatrix(const glm::mat4& matrix) noexcept {
	mMat = matrix;
	if (mpFirstChild) {
		mpFirstChild->updateMatrix(mParentWorldMat * mMat);
	}
}


void Node::translate(float x, float y, float z) noexcept {
	glm::mat4 temp{ 1.0f };
	mMat = glm::translate(temp, glm::vec3(x, y, z)) * mMat;
	if (mpFirstChild) {
		mpFirstChild->updateMatrix(mParentWorldMat * mMat);
	}
}

void Node::rotate(float angle, const glm::vec3& vec) noexcept {
	glm::mat4 temp{ 1.0f };
	mMat = glm::rotate(temp, angle, vec) * mMat;
	if (mpFirstChild) {
		mpFirstChild->updateMatrix(mParentWorldMat * mMat);
	}
}

void Node::scale(const glm::vec3& scaleVec) noexcept {
	glm::mat4 temp{ 1.0f };
	mMat = glm::scale(temp, scaleVec) * mMat;
	if (mpFirstChild) {
		mpFirstChild->updateMatrix(mParentWorldMat * mMat);
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

void Node::lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up) noexcept {
	mMat = glm::lookAt(eyepos, center, up);
	if (mpFirstChild) {
		mpFirstChild->updateMatrix(mParentWorldMat*mMat);
	}
}

void Node::addListener(const shared_ptr<NodeListener>& lis) {
	mListeners.emplace_back(lis);
}

void Node::updateMatrix(glm::mat4& mat) noexcept {
	setParentWorldMatrix(mat);
	//更新pNode的兄弟节点
	if (mpNextSibling) {
		mpNextSibling->updateMatrix(mat);
	}
	if (mpFirstChild) {
		//更新pNode的子节点
		mpFirstChild->updateMatrix(mParentWorldMat * mMat);
	}
	for (auto& listener : mListeners) {
		if (listener) {
			listener->update(mat);
		}
	}
	
}