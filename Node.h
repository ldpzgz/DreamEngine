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
	Node(Node&& temp);һ��Ҫ�޸�temp�������ƶ����캯��û������*/
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

	void setMatrix(glm::mat4& matrix);

	glm::mat4& getMatrix() {
		return mMat;
	}

	void getWorldMatrix(glm::mat4&);

	MapIMesh& getMeshes() {
		return mMeshes;
	}

	MapINode& getChildren() {
		return mChildren;
	}

	virtual void translate(float x,float y,float z);

	virtual void lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up);
protected:
	glm::mat4 mMat;
private:
	unsigned int mId;
	atomic_uint mCurChileId;
	atomic_uint mCurMeshId;
	weak_ptr<Node> mpParent;	//ʹ��weak_ptr��ֹ����nodeѭ�����õ����ڴ�й©
	unordered_map<unsigned int, shared_ptr<Node>> mChildren;
	unordered_map<unsigned int, shared_ptr<Mesh>> mMeshes;

	void setParent(shared_ptr<Node>&);
	void setId(unsigned int id) {
		mId = id;
	}
};
#endif