#ifndef _NODE_H_
#define _NODE_H_
#include <atomic>
#include <unordered_map>
#include <vector>
#include <list>
#include <memory>
#include <any>
#include <functional>
#include <glm/vec3.hpp>           // vec3
#include <glm/mat4x4.hpp>         // mat4
using namespace std;

class Renderable;
class NodeListener;
enum class NodeAnyIndex {
	UiTreeIndex
};
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

	auto& getChildren() noexcept {
		return mChildren;
	}

	void addChild(shared_ptr<Node>& child);

	void removeChild(shared_ptr<Node>& child) noexcept;

	bool addRenderable(const shared_ptr<Renderable>& temp);

	void setLocalMatrix(const glm::mat4& matrix,bool updateChild = true,bool notify=true) noexcept;
	
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

	void setParentWorldMatrix(const glm::mat4& matrix, bool updateChild = true, bool notify = true) noexcept;

	const auto& getRenderables() const noexcept {
		return mRenderables;
	}

	/*
	* construct a moveMat
	* final matrix：moveMat * mLocalMat
	*/
	virtual void translate(float x, float y, float z, bool updateChild = true, bool notify = true) noexcept;

	/*
	* construct a rotateMat: rotate angle around aix vec
	* final matrix：rotateMat*mLocalMat
	*/
	virtual void rotate(float angle, const glm::vec3& vec, bool updateChild = true, bool notify = true) noexcept;
	/*
	* construct a scaleMat
	* final matrix：mMat*scaleMat
	*/
	void scale(const glm::vec3& scaleVec, bool updateChild = true, bool notify = true) noexcept;

	/*
	* construct a lookAtMat
	* final matrix：mMat=lookMat
	*/
	virtual void lookAt(const glm::vec3& eyepos, const glm::vec3& center, 
		const glm::vec3& up, bool updateChild = true, bool notify = true) noexcept;


	void visitNode(const std::function<void(Node*)>& func);

	void visitNode(const std::function<bool(Node*)>& func,bool& isOver);

	void visitNode(const std::function<bool(Node*,bool& visitChild)>& func, bool& isOver);
		
	void addListener(const shared_ptr<NodeListener>& lis);
	
	//you can attachment anything to the node
	void addAny(int index,const std::any& a) {
		mAttachments[index] = a;
	}
	std::any& getAny(int index) {
		static std::any sAny;
		auto it = mAttachments.find(index);
		if (it != mAttachments.end()) {
			return it->second;
		}
		else {
			return sAny;
		}
	}
	//void removeListener();
protected:
	glm::mat4 mLocalMat{1.0f};
	glm::mat4 mParentWorldMat{1.0f};
private:
	void updateListener();
	void updateChildrenMatrix(bool notify=true) noexcept;
	void setParent(shared_ptr<Node>& parent) noexcept {
		mpParent = parent;
	}

	std::string mName;
	weak_ptr<Node> mpParent;
	static atomic_uint sCurMeshId;
	unordered_map<unsigned int, shared_ptr<Renderable>> mRenderables;
	std::list<std::shared_ptr<Node>> mChildren;
	std::vector<shared_ptr<NodeListener>> mListeners;
	std::unordered_map<int, std::any> mAttachments;
};
using NodeSP = std::shared_ptr<Node>;
#endif