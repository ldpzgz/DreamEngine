#ifndef _NODE_H_
#define _NODE_H_
#include <unordered_map>
#include <memory>
#include <atomic>
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/mat4x4.hpp>
#include "Mesh.h"
using namespace std;
class Node : enable_shared_from_this<Node> {
public:
	Node();
	virtual ~Node();
	/*Node(Node&);
	Node(const Node&);
	Node(Node&& temp);一定要修改temp，否则移动构造函数没有意义*/
	shared_ptr<Node> newAChild();
	bool hasParent();
	shared_ptr<Node> getParent() {
		return mpParent.lock();
	}
	bool addChild(shared_ptr<Node>&);
	bool removeChild(unsigned int childId);
	bool addMesh(shared_ptr<Mesh>&);

	unsigned int getId() {
		return mId;
	}

	void setMatrix(glm::mat4& matrix);

	glm::mat4& getMatrix() {
		return mMat;
	}
private:
	unsigned int mId;
	atomic_uint mCurChileId;
	atomic_uint mCurMeshId;
	glm::mat4 mMat;
	weak_ptr<Node> mpParent;	//使用weak_ptr防止父子node循环引用导致内存泄漏
	unordered_map<unsigned int, shared_ptr<Node>> mChildren;
	unordered_map<unsigned int, shared_ptr<Mesh>> mMeshes;

	void setParent(shared_ptr<Node>&);
	void setId(unsigned int id) {
		mId = id;
	}
};
#endif