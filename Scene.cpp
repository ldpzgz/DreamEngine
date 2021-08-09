#include"Scene.h"
#include "Camera.h"
Scene::Scene() {
	mpRootNode = make_shared<Node<glm::mat4>>();
}
Scene::~Scene() {

}

shared_ptr<Camera> Scene::createACamera() {
	auto camera = make_shared<Camera>(shared_from_this());
	mCameras.push_back(camera);
	mpRootNode->addChild(static_pointer_cast<Node<glm::mat4>>(camera));
	return camera;
}