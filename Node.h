#ifndef _NODE_H_
#define _NODE_H_
#include <unordered_map>
#include <vector>
#include <memory>
#include <atomic>
#include <functional>

#include <glm/vec3.hpp>           // vec3
#include <glm/mat4x4.hpp>         // mat4

#include <type_traits>
using namespace std;

/*
glm这个库在不设置任何宏的情况下，矩阵相乘与书本上的顺序一致
glm::mat4 modelMat;
modelMat[0],modelMat[1],modelMat[2],modelMat[3],这是个东西
不管设置的是左手坐标系还是右手坐标系，都表示x,y,z,w轴
x,y,z是负责旋转的那三根轴，w是负责平移的
*/

class Renderable;
class NodeListener;

class Node : public enable_shared_from_this<Node> {
public:
	Node();

	virtual ~Node();

	void setId(int id) noexcept{
		mId = id;
	}

	unsigned int getId() const noexcept{
		return mId;
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


	void setMatrix(const glm::mat4& matrix) noexcept;

	//只返回我自己的matrix
	const glm::mat4& getMatrix() const noexcept {
		return mMat;
	}

	glm::mat4& getMatrix() noexcept {
		return mMat;
	}

	glm::mat4 getWorldMatrix() const noexcept {
		return mParentWorldMat*mMat;
	}

	const glm::mat4& getParentWorldMat() {
		return mParentWorldMat;
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
	* 根据x,y,z构造一个moveMat
	* 最终矩阵：mMat*moveMat
	* 效果：v*mMat*moveMat
	*/
	virtual void translate(float x, float y, float z) noexcept;

	/*
	* 根据angle,vec构造一个rotateMat
	* 最终矩阵：mMat*rotateMat
	* 效果：v*mMat*rotateMat
	*/
	virtual void rotate(float angle, const glm::vec3& vec) noexcept;

	/*
	* 根据scaleVec构造一个scaleMat
	* 最终矩阵：mMat*scaleMat
	* 效果：v*mMat*scaleMat
	*/
	void scale(const glm::vec3& scaleVec) noexcept;

	void visitNode(const std::function<void(Node*)>& func);

	/*
	* 根据eyepos,center,up构造一个lookMat
	* 最终mMat矩阵与直觉相反：mMat=lookMat
	* 效果：v*lookMat
	*/
	virtual void lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up) noexcept;
	
	void addListener(const shared_ptr<NodeListener>& lis);
	
	//void removeListener();
protected:
	glm::mat4 mMat{1.0f};
	glm::mat4 mParentWorldMat{1.0f};
	void setParentWorldMatrix(const glm::mat4& matrix) noexcept {
		mParentWorldMat = matrix;
	}
	virtual void updateMatrix( glm::mat4& mat)noexcept;
private:
	unsigned int mId;
	weak_ptr<Node> mpParent;	//使用weak_ptr防止父子node循环引用导致内存泄漏
	unordered_map<unsigned int, shared_ptr<Renderable>> mRenderables;
	shared_ptr<Node> mpFirstChild;
	shared_ptr<Node> mpLastChild;
	shared_ptr<Node> mpNextSibling;
	weak_ptr<Node> mpPreSibling;
	std::vector<shared_ptr<NodeListener>> mListeners;

	static atomic_uint sCurChildId;
	static atomic_uint sCurMeshId;

	void setParent(shared_ptr<Node>& parent) noexcept{
		mpParent = parent;
	}
};
using NodeSP = std::shared_ptr<Node>;
#endif