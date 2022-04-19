#ifndef _NODE_H_
#define _NODE_H_
#include <atomic>
#include <unordered_map>
#include <vector>
#include <memory>
#include <functional>
#include <glm/vec3.hpp>           // vec3
#include <glm/mat4x4.hpp>         // mat4
using namespace std;

class Renderable;
class NodeListener;
class Node : public enable_shared_from_this<Node> {
public:
	Node();
	virtual ~Node();

	void setName(const std::string& name) noexcept{
		mName = name;
	}

	const std::string& getName() const noexcept{
		return mName;
	}
	
	shared_ptr<Node> newAChild();

	bool hasParent() const noexcept{
		return !mpParent.expired();
	}

	auto& getParent() const noexcept{
		return mpParent;
	}

	bool addChild(shared_ptr<Node>& child);

	bool removeChild(shared_ptr<Node>& child) noexcept;

	bool addRenderable(const shared_ptr<Renderable>& temp);

	void setLocalMatrix(const glm::mat4& matrix) noexcept;

	void setLocalMatrixOnlyChild(const glm::mat4& matrix) noexcept;

	const glm::mat4& getLocalMatrix() const noexcept {
		return mLocalMat;
	}

	glm::mat4& getLocalMatrix() noexcept {
		return mLocalMat;
	}

	glm::mat4 getWorldMatrix() const noexcept {
		return mParentWorldMat*mLocalMat;
	}

	const glm::mat4& getParentWorldMat() noexcept {
		return mParentWorldMat;
	}

	void setParentWorldMatrix(const glm::mat4& matrix) noexcept {
		mParentWorldMat = matrix;
	}

	const auto& getRenderables() const noexcept {
		return mRenderables;
	}

	auto& getFirstChild() const noexcept {
		return mpFirstChild;
	}
	auto& getNextSibling() const noexcept {
		return mpNextSibling;
	}
	/*
	* construct a moveMat
	* final matrix：moveMat * mLocalMat
	*/
	virtual void translate(float x, float y, float z) noexcept;

	/*
	* construct a rotateMat: rotate angle around aix vec
	* final matrix：rotateMat*mLocalMat
	*/
	virtual void rotate(float angle, const glm::vec3& vec) noexcept;

	/*
	* construct a scaleMat
	* final matrix：mMat*scaleMat
	*/
	void scale(const glm::vec3& scaleVec) noexcept;

	void visitNode(const std::function<void(Node*)>& func);

	void visitNode(const std::function<bool(Node*)>& func,bool& isOver);
	/*
	* construct a lookAtMat
	* final matrix：mMat=lookMat
	*/
	virtual void lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up) noexcept;
	
	void addListener(const shared_ptr<NodeListener>& lis);
	
	//void removeListener();
protected:
	glm::mat4 mLocalMat{1.0f};
	glm::mat4 mParentWorldMat{1.0f};
	void updateMatrixHierarchy( glm::mat4& parentMat) noexcept;
	void updateMatrixMeAndSibling(glm::mat4& parentMat) noexcept;
private:
	std::string mName;
	weak_ptr<Node> mpParent;
	static atomic_uint sCurMeshId;
	unordered_map<unsigned int, shared_ptr<Renderable>> mRenderables;
	shared_ptr<Node> mpFirstChild;
	shared_ptr<Node> mpLastChild;
	shared_ptr<Node> mpNextSibling;
	weak_ptr<Node> mpPreSibling;
	std::vector<shared_ptr<NodeListener>> mListeners;

	void setParent(shared_ptr<Node>& parent) noexcept{
		mpParent = parent;
	}
};
using NodeSP = std::shared_ptr<Node>;
#endif