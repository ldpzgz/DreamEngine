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

void Node::setAny(NodeAnyIndex index, const std::any & a) {
	mAttachments[index] = a;
}

std::any Node::getAny(NodeAnyIndex index) {
	return mAttachments[index];
}

shared_ptr<Node> Node::newAChild() {
	auto child = make_shared<Node>();
	addChild(child);
	return child;
}

glm::mat4 Node::getWorldMatrix() noexcept {
	if (mbHasNodeAnimation) {
		try {
			auto& nodeMat = std::any_cast<glm::mat4>(mAttachments[NodeAnyIndex::NodeAnimationMatrix]);
			return nodeMat * mParentWorldMat * mLocalMat;
		}
		catch (std::bad_any_cast e) {
			LOGE("Node::getWorldMatrix() bad any cast");
			return mParentWorldMat * mLocalMat;
		}
	}
	else {
		return mParentWorldMat * mLocalMat;
	}
}

void Node::addChild(shared_ptr<Node>& child) {
	if (child) {
		child->setParent(shared_from_this());
		child->setParentWorldMatrix(getWorldMatrix());
		child->mIdInParent = mChildren.size();
		mChildren.emplace_back(child);
	}
}

void Node::removeChild(shared_ptr<Node>& child) noexcept{
	if (child) {
		auto parent = child->getParent().lock();
		if (parent.get() == this) {
			mChildren.erase(std::cbegin(mChildren) + child->mIdInParent);
		}
	}
}

void Node::removeChild(int index) noexcept {
	if (index < mChildren.size()) {
		mChildren.erase(std::cbegin(mChildren) + index);
	}
}

bool Node::addRenderable(const shared_ptr<Renderable>& temp) {
	unsigned int rid = sCurMeshId++;
	temp->setRid(rid);
	mRenderables.emplace(rid, temp);
	return true;
}

void Node::setParentWorldMatrix(const glm::mat4& matrix, bool updateAllChildren,bool notify) noexcept {
	mParentWorldMat = matrix;
	if (notify) {
		updateListener();
	}
	if (updateAllChildren) {
		updateAllChild(notify);
	}
	else {
		updateDirectChild(notify);
	}
}

void Node::setLocalMatrix(const glm::mat4& matrix,bool updateChild, bool notify) noexcept {
	mLocalMat = matrix;
	if (notify) {
		updateListener();
	}
	if (updateChild) {
		updateAllChild(notify);
	}
	else {
		updateDirectChild(notify);
	}
}

void Node::translate(float x, float y, float z, bool updateChild, bool notify) noexcept {
	glm::mat4 temp{ 1.0f };
	mLocalMat = glm::translate(temp, glm::vec3(x, y, z)) * mLocalMat;
	if (notify) {
		updateListener();
	}
	if (updateChild) {
		updateAllChild(notify);
	}
	else {
		updateDirectChild(notify);
	}
}

void Node::rotate(float angle, const glm::vec3& vec, bool updateChild, bool notify) noexcept {
	glm::mat4 temp{ 1.0f };
	mLocalMat = glm::rotate(temp, angle, vec) * mLocalMat;
	if (notify) {
		updateListener();
	}
	if (updateChild) {
		updateAllChild(notify);
	}
	else {
		updateDirectChild(notify);
	}
}

void Node::scale(const glm::vec3& scaleVec, bool updateChild, bool notify) noexcept {
	glm::mat4 temp{ 1.0f };
	mLocalMat = glm::scale(temp, scaleVec) * mLocalMat;
	if (notify) {
		updateListener();
	}
	if (updateChild) {
		updateAllChild(notify);
	}
	else {
		updateDirectChild(notify);
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

void Node::visitNode(const std::function<void(Node*, bool& visitChild)>& func) {
	bool visitChild = true;
	if (func) {
		func(this, visitChild);
	}
	if (!visitChild) {
		return;
	}
	for (auto& pChild : mChildren) {
		if (pChild) {
			pChild->visitNode(func);
		}
	}
}

void Node::visitNode(const std::function<bool(Node*, bool& visitChild)>& func, bool& isOver) {
	bool visitChild = true;
	if (func) {
		isOver = func(this, visitChild);
	}
	if (isOver || !visitChild) {
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

void Node::visitNodeForward(const std::function<bool(Node*)>& func,
	const std::function<bool(Node*)>& visitChildFunc,
	bool& isOver) {
	isOver = func(this);
	if (isOver)
		return;
	if (!mChildren.empty() && visitChildFunc(this)) {
		for (auto it = mChildren.begin(); it != mChildren.end(); ++it) {
			auto& pNode = *it;
			if (pNode) {
				(pNode)->visitNodeForward(func, visitChildFunc, isOver);
			}
			if (isOver)
				break;
		}
	}
}

void Node::visitNodeBackward(const std::function<bool(Node*)>& func,
	const std::function<bool(Node*)>& visitChildFunc,
	bool& isOver) {

	if (!mChildren.empty() && visitChildFunc(this)) {
		for (auto it = mChildren.rbegin(); it != mChildren.rend(); ++it) {
			if (isOver)
				break;
			auto& pNode = *it;
			if (pNode) {
				pNode->visitNodeBackward(func, visitChildFunc, isOver);
			}
		}
	}
	
	if (isOver)
		return;
	isOver = func(this);
}

void Node::lookAt(const glm::vec3& eyepos, const glm::vec3& center, 
	const glm::vec3& up, bool updateChild, bool notify) noexcept {
	mLocalMat = glm::lookAt(eyepos, center, up);
	if (notify) {
		updateListener();
	}
	if (updateChild) {
		updateAllChild(notify);
	}
	else {
		updateDirectChild(notify);
	}
}

void Node::addListener(const shared_ptr<NodeListener>& lis) {
	mListeners.emplace_back(lis);
}

void Node::updateDirectChild(bool notify) {
	auto mat = mParentWorldMat * mLocalMat;
	for (auto& pNode : mChildren) {
		pNode->mParentWorldMat = mat;
	}
}
void Node::updateAllChild(bool notify) {
	auto mat = mParentWorldMat * mLocalMat;
	for (auto& pNode : mChildren) {
		pNode->setParentWorldMatrix(mat,true,notify);
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