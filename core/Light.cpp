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

void Light::draw(const glm::mat4* modelMat,
	const glm::mat4* texMat,
	const glm::mat4* projViewMat) {

}