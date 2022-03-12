#ifndef _NODE_H_
#define _NODE_H_
#include <unordered_map>
#include <vector>
#include <memory>
#include <atomic>

#include <glm/vec3.hpp>           // vec3
#include <glm/mat4x4.hpp>         // mat4

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

class Renderable;

/*
* 节点方位变化监听
*/
class NodeListener {
public:
	virtual void update(const glm::mat4& mat) = 0;
};

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

	bool removeChild(unsigned int childId) noexcept;

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

	const auto& getRenderables() const noexcept {
		return mRenderables;
	}

	auto& getChildren() const noexcept {
		return mChildren;
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

	/*
	* 根据eyepos,center,up构造一个lookMat
	* 最终mMat矩阵与直觉相反：mMat=lookMat
	* 效果：v*lookMat
	*/
	virtual void lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up) noexcept;
	
	void addListener(const shared_ptr<NodeListener>& lis);
	
	//void removeListener();
protected:
	glm::mat4 mMat;
	glm::mat4 mParentWorldMat;
	void setParentWorldMatrix(const glm::mat4& matrix) noexcept {
		mParentWorldMat = matrix;
	}
	void updateChildWorldMatrix()noexcept;
private:
	unsigned int mId;
	weak_ptr<Node> mpParent;	//使用weak_ptr防止父子node循环引用导致内存泄漏
	unordered_map<unsigned int, shared_ptr<Node>> mChildren;
	unordered_map<unsigned int, shared_ptr<Renderable>> mRenderables;
	std::vector<shared_ptr<NodeListener>> mListeners;

	static atomic_uint sCurChildId;
	static atomic_uint sCurMeshId;

	void setParent(shared_ptr<Node>& parent) noexcept{
		mpParent = parent;
	}
};
using NodeSP = std::shared_ptr<Node>;
#endif