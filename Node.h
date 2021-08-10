#ifndef _NODE_H_
#define _NODE_H_
#include <unordered_map>
#include <memory>
#include <atomic>
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
// Include all GLM extensions
#include <glm/ext.hpp> // perspective, translate, rotate
#include <glm/gtx/matrix_transform_2d.hpp>
#include "Mesh.h"
#include <type_traits>
using namespace std;
template<typename T>
class Node : public enable_shared_from_this<Node<T>> {
public:
	using WeakNode = weak_ptr<Node>;
	using MapINode = unordered_map<unsigned int, shared_ptr<Node>>;
	using MapIAttachable = unordered_map<unsigned int, shared_ptr<Attachable>>;

	Node() :
		mCurChileId(0),
		mCurMeshId(0),
		mMat(1.0f),
		mParentWorldMat(1.0f),
		mId(0)
	{

	}

	virtual ~Node() {

	}
	/*Node(Node&);
	Node(const Node&);
	Node(Node&& temp);һ��Ҫ�޸�temp�������ƶ����캯��û������*/
	shared_ptr<Node<T>> newAChild() {
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

	bool hasParent() {
		return !mpParent.expired();
	}


	
	WeakNode& getParent() {
		return mpParent;
	}

	bool addChild(shared_ptr<Node<T>>& child) {
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

	bool removeChild(unsigned int childId) {
		return mChildren.erase(childId) == 1 ? true : false;
	}

	bool addAttachment(const shared_ptr<Attachable>& temp) {
		return mAttachments.try_emplace(mCurMeshId++, temp).second;
	}


	unsigned int getId() {
		return mId;
	}

	void setMatrix(const T& matrix) noexcept {
		mMat = matrix;
		updateChildWorldMatrix();
	}


	T& getMatrix() {
		return mMat;
	}

	T getWorldMatrix() {
		return mParentWorldMat*mMat;
	}

	MapIAttachable& getAttachments() {
		return mAttachments;
	}

	MapINode& getChildren() {
		return mChildren;
	}


	//�麯����ģ����ʵ������ʱ��û�е��ù�Ҳ����������麯��
	void translate(float x,float y,float z) {
		mMat = glm::translate(mMat, glm::vec3(x, y, z));
		updateChildWorldMatrix();
	}

	//ֻ��Node<glm::mat3>���ܵ���
	void translate(float x, float y) {
		mMat = glm::translate(mMat, glm::vec2(x, y));
		updateChildWorldMatrix();
	}

	void rotate(float angle,const glm::vec3& vec) {
		mMat = glm::rotate(mMat, angle, vec);
		updateChildWorldMatrix();
	}

	//ֻ��Node<glm::mat3>���ܵ���
	void rotate(float angle, const glm::vec2& vec) {
		mMat = glm::rotate(mMat, angle, vec);
		updateChildWorldMatrix();
	}

	void scale(const glm::vec3& scaleVec) {
		mMat = glm::scale(mMat, scaleVec);
		updateChildWorldMatrix();
	}

	//ֻ��Node<glm::mat3>���ܵ���
	void scale(const glm::vec2& scaleVec) {
		mMat = glm::scale(mMat, scaleVec);
		updateChildWorldMatrix();
	}


	virtual void lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up) {
		mMat = glm::lookAt(eyepos, center, up);
		updateChildWorldMatrix();
	}
protected:
	T mMat;
	T mParentWorldMat;
private:
	unsigned int mId;
	atomic_uint mCurChileId;
	atomic_uint mCurMeshId;
	weak_ptr<Node<T>> mpParent;	//ʹ��weak_ptr��ֹ����nodeѭ�����õ����ڴ�й©
	unordered_map<unsigned int, shared_ptr<Node<T>>> mChildren;
	unordered_map<unsigned int, shared_ptr<Attachable>> mAttachments;

	void setParent(shared_ptr<Node<T>>& parent) {
		mpParent = parent;
	}

	void setId(unsigned int id) {
		mId = id;
	}

	template<typename T>
	void setParentWorldMatrix(const T& matrix) noexcept {
		mParentWorldMat = matrix;
	}

	void updateChildWorldMatrix() const noexcept {
		if (!mChildren.empty()) {
			auto myWorldMat = mParentWorldMat* mMat;
			for (auto& child : mChildren) {
				child.second->setParentWorldMatrix(myWorldMat);
				child.second->updateChildWorldMatrix();
			}
			/*std::for_each(mChildren.begin(), mChildren.end(), [&myWorldMat](const MapINode::value_type child) {
				child.second->setParentWorldMatrix(myWorldMat);
				child.second->updateChildWorldMatrix();
			});*/
		}
	}
};
#endif