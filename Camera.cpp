#include "Camera.h"
#include "Scene.h"
#include <glm/gtc/matrix_transform.hpp>
Camera::Camera(shared_ptr<Scene> ps):
	mpScene(ps),
	mProj(1.0f),
	aspect(4.0f / 3.0f),
	fov(45.0f),
	nearp(0.1f),
	farp(100.0f)
{
	perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
}

Camera::Camera(shared_ptr<Scene> ps, float asp) :
	mpScene(ps),
	mProj(1.0f),
	aspect(asp),
	fov(45.0f),
	nearp(0.1f),
	farp(100.0f)
{
	perspective(fov, aspect,nearp,farp);
}

Camera::~Camera()
{
}

void Camera::perspective(float fovy, float asp, float n, float f) {
	mProj = glm::perspective(fovy, asp,n,f);
	aspect = asp;
	fov = fovy;
	nearp = n;
	farp = f;
}

void Camera::renderScene() {
	auto scene = mpScene.lock();
	if (scene) {

	}
}

void Camera::setAspect(float asp) {
	aspect = asp;
	perspective(fov, aspect, nearp, farp);
}