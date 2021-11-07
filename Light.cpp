#include "Light.h"

Light::Light(LightType type) :
	Node<glm::mat4>(),
	mType(type)
{

}

Light::Light() :
	Node<glm::mat4>() {

}

Light::~Light() {

}