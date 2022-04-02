#ifndef _RENDERABLE_H_
#define _RENDERABLE_H_
#include <vector>
#include <glm/vec3.hpp>           // vec3
#include <glm/mat4x4.hpp>         // mat4
class Renderable {
public:
	virtual ~Renderable() {

	}
	virtual void draw(int posloc, int texloc = -1, int norloc = -1, int colorloc = -1, int tangentloc = -1) {

	}
	virtual void draw(const glm::mat4* modelMat,
		const glm::mat4* texMat = nullptr,
		const glm::mat4* projViewMat = nullptr) = 0;

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
