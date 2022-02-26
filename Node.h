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

/*
glm这个库的矩阵运算顺序：
glm::mat4 model(1.0f);
model = glm::translate(model,glm::vec3(10,10,0));
model = glm::scale(model,glm::vec3(2.0f,2.0f,0.0f);
glm::vec4 temp(1.0f,1.0f,0.0f,1.0f);
auto result = temp * model;

result 是先在原点缩放，然后再平移，glm这个库矩阵运算与之前认识的运算顺序相反。
*/

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
	Node(Node&& temp);一定要修改temp，否则移动构造函数没有意义*/
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

	//只返回我自己的matrix
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


	//虚函数，模板在实例化的时候，没有调用过也会生成这个虚函数
	void translate(float x,float y,float z) {
		mMat = glm::translate(mMat, glm::vec3(x, y, z));
		updateChildWorldMatrix();
	}

	//只有Node<glm::mat3>才能调用
	void translate(float x, float y) {
		mMat = glm::translate(mMat, glm::vec2(x, y));
		updateChildWorldMatrix();
	}

	void rotate(float angle,const glm::vec3& vec) {
		mMat = glm::rotate(mMat, angle, vec);
		updateChildWorldMatrix();
	}

	//只有Node<glm::mat3>才能调用
	void rotate(float angle, const glm::vec2& vec) {
		mMat = glm::rotate(mMat, angle, vec);
		updateChildWorldMatrix();
	}

	void scale(const glm::vec3& scaleVec) {
		mMat = glm::scale(mMat, scaleVec);
		updateChildWorldMatrix();
	}

	//只有Node<glm::mat3>才能调用
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
	weak_ptr<Node<T>> mpParent;	//使用weak_ptr防止父子node循环引用导致内存泄漏
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

using SP_Node = std::shared_ptr<Node<glm::mat4>>;
#endif