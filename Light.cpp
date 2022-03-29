#include "Rect.h"
#include "Light.h"

Light::Light(LightType type) :
	mType(type)
{

}

Light::Light(LightType type, bool bCastShadow):
	mType(type),
	mbCastShadow(bCastShadow)
{

}

void Light::draw(const glm::mat4* projMat,
	const glm::mat4* modelMat,
	const glm::mat4* viewMat,
	const glm::mat4* texMat,
	const std::vector<glm::vec3>* lightPos,
	const std::vector<glm::vec3>* lightColor,
	const glm::vec3* viewPos) {

}