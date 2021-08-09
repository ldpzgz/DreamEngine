#include "Node.h"
#include "Log.h"

template<typename T>
Node<T>::Node():
	mCurChileId(0),
	mCurMeshId(0),
	mMat(1.0f),
	mParentWorldMat(1.0f),
	mId(0)
{

}

template<typename T>
Node<T>::~Node() {

}

template<typename T>
shared_ptr<Node<T>> Node<T>::newAChild() {
	auto child = make_shared<Node<T>>();
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
template<typename T>
bool Node<T>::hasParent() {
	return !mpParent.expired();
}

template<typename T>
void Node<T>::setParent(shared_ptr<Node<T>>& parent) {
	mpParent = parent;
}

template<typename T>
bool Node<T>::removeChild(unsigned int childId) {
	return mChildren.erase(childId) == 1 ? true : false;
}

template<typename T>
bool Node<T>::addChild(shared_ptr<Node<T>>& child) {
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

template<typename T>
bool Node<T>::addMesh(shared_ptr<Mesh>& temp) {
	return mMeshes.try_emplace(mCurMeshId++,temp).second;
}

template<typename T>
void Node<T>::setMatrix(const T& matrix) noexcept {
	mMat = matrix;
	updateChildWorldMatrix();
}

template<typename T>
void Node<T>::updateChildWorldMatrix() const noexcept {
	if (!mChildren.empty()) {
		auto myWorldMat = mParentWorldMat* mMat;
		std::for_each(mChildren.begin(), mChildren.end(), [&myWorldMat](const MapINode::value_type child) {
			child.second->setParentWorldMatrix(myWorldMat);
			child.second->updateChildWorldMatrix();
		});
	}
}

template<typename T>
void Node<T>::translate(float x, float y, float z) {
	mMat = glm::translate(mMat, glm::vec3(x, y, z));
	updateChildWorldMatrix();
}

template<typename T>
void Node<T>::translate(float x, float y) {
	mMat = glm::translate(mMat, glm::vec2(x, y));
	updateChildWorldMatrix();
}

template<typename T>
void Node<T>::lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up) {
	mMat = glm::lookAt(eyepos, center, up);
	updateChildWorldMatrix();
}

template<typename T>
void Node<T>::rotate(float angle, const glm::vec3& vec) {
	mMat = glm::rotate(mMat, angle, vec);
	updateChildWorldMatrix();
}

template<typename T>
void Node<T>::rotate(float angle, const glm::vec2& vec) {
	mMat = glm::rotate(mMat, angle, vec);
	updateChildWorldMatrix();
}

template<typename T>
void Node<T>::scale(const glm::vec3& scaleVec) {
	mMat = glm::scale(mMat, scaleVec);
	updateChildWorldMatrix();
}

template<typename T>
void Node<T>::scale(const glm::vec2& scaleVec) {
	mMat = glm::scale(mMat, scaleVec);
	updateChildWorldMatrix();
}