#ifndef _NODE_H_
#define _NODE_H_
#include <map>
#include <memory>
#include <atomic>
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/mat4x4.hpp>
using namespace std;
class Node : enable_shared_from_this<Node> {
public:
	Node();
	shared_ptr<Node> newAChild();
	bool setParent(shared_ptr<Node>);
	void addChild(shared_ptr<Node>);
private:
	atomic_uint mCurChileId;
	glm::mat4 mMat;
	shared_ptr<Node> mpParent;
	map<int, shared_ptr<Node>> mChildren;
};
#endif