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

class Attachable;
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

	bool addAttachment(const shared_ptr<Attachable>& temp);


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

	const auto& getAttachments() const noexcept {
		return mAttachments;
	}

	auto& getChildren() const noexcept {
		return mChildren;
	}

	void translate(float x, float y, float z) noexcept;

	void rotate(float angle, const glm::vec3& vec) noexcept;

	void scale(const glm::vec3& scaleVec) noexcept;


	virtual void lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up) noexcept;
protected:
	glm::mat4 mMat;
	glm::mat4 mParentWorldMat;
private:
	unsigned int mId;
	weak_ptr<Node> mpParent;	//使用weak_ptr防止父子node循环引用导致内存泄漏
	unordered_map<unsigned int, shared_ptr<Node>> mChildren;
	unordered_map<unsigned int, shared_ptr<Attachable>> mAttachments;

	static atomic_uint sCurChildId;
	static atomic_uint sCurMeshId;

	void setParent(shared_ptr<Node>& parent) noexcept{
		mpParent = parent;
	}

	void setParentWorldMatrix(const glm::mat4& matrix) noexcept {
		mParentWorldMat = matrix;
	}

	void updateChildWorldMatrix() noexcept;
};
using NodeSP = std::shared_ptr<Node>;
#endif