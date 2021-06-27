#include "Camera.h"
#include "Scene.h"
#include <glm/gtc/matrix_transform.hpp>
Camera::Camera(shared_ptr<Scene> ps):
	mpScene(ps),
	mProj(1.0f)
{
	perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
}

Camera::~Camera()
{
}

void Camera::perspective(float fovy, float aspect, float n, float f) {
	mProj = glm::perspective(fovy, aspect,n,f);
}

void Camera::renderScene() {
	auto scene = mpScene.lock();
	if (scene) {

	}
}