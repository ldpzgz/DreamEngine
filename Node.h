#ifndef _NODE_H_
#define _NODE_H_
#include <unordered_map>
#include <memory>
#include <atomic>
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/mat4x4.hpp>
#include "Mesh.h"
using namespace std;
class Node : public enable_shared_from_this<Node> {
public:
	using WeakNode = weak_ptr<Node>;
	using MapINode = unordered_map<unsigned int, shared_ptr<Node>>;
	using MapIMesh = unordered_map<unsigned int, shared_ptr<Mesh>>;

	Node();
	virtual ~Node();
	/*Node(Node&);
	Node(const Node&);
	Node(Node&& temp);一定要修改temp，否则移动构造函数没有意义*/
	shared_ptr<Node> newAChild();
	bool hasParent();

	
	WeakNode& getParent() {
		return mpParent;
	}

	bool addChild(shared_ptr<Node>&);
	bool removeChild(unsigned int childId);
	bool addMesh(shared_ptr<Mesh>&);

	unsigned int getId() {
		return mId;
	}

	void setMatrix(const glm::mat4& matrix) noexcept{
		mMat = matrix;
		updateChildWorldMatrix();
	}

	glm::mat4& getMatrix() {
		return mMat;
	}

	glm::mat4 getWorldMatrix() {
		return mParentWorldMat*mMat;
	}

	MapIMesh& getMeshes() {
		return mMeshes;
	}

	MapINode& getChildren() {
		return mChildren;
	}



	virtual void translate(float x,float y,float z);
	virtual void rotate(float angle,const glm::vec3& vec);
	virtual void scale(const glm::vec3& scaleVec);
	virtual void lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up);
protected:
	glm::mat4 mMat;
	glm::mat4 mParentWorldMat;
private:
	unsigned int mId;
	atomic_uint mCurChileId;
	atomic_uint mCurMeshId;
	weak_ptr<Node> mpParent;	//使用weak_ptr防止父子node循环引用导致内存泄漏
	unordered_map<unsigned int, shared_ptr<Node>> mChildren;
	unordered_map<unsigned int, shared_ptr<Mesh>> mMeshes;

	void setParent(shared_ptr<Node>&);
	void setId(unsigned int id) {
		mId = id;
	}

	void setParentWorldMatrix(const glm::mat4& matrix) noexcept {
		mParentWorldMat = matrix;
	}
	void updateChildWorldMatrix() const noexcept;
};
#endif