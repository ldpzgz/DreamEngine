#ifndef _RENDERABLE_H_
#define _RENDERABLE_H_
#include <vector>
#include <glm/vec3.hpp>           // vec3
#include <glm/mat4x4.hpp>         // mat4
class Renderable {
public:
	virtual ~Renderable() {

	}

	virtual void draw(const glm::mat4* projMat, 
		const glm::mat4* modelMat, 
		const glm::mat4* viewMat = nullptr,
		const glm::mat4* texMat = nullptr,
		const std::vector<glm::vec3>* lightPos = nullptr, 
		const std::vector<glm::vec3>* lightColor = nullptr, 
		const glm::vec3* viewPos = nullptr) = 0;

	unsigned int getRid() {
		return mId;
	}
	void setRid(unsigned int id) {
		mId = id;
	}
private:
	unsigned int mId;
};

#endif
