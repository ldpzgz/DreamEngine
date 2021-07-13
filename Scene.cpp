#include"Scene.h"
#include "Camera.h"
Scene::Scene() {
	mpRootNode = make_shared<Node>();
}
Scene::~Scene() {

}

shared_ptr<Camera> Scene::createACamera() {
	auto camera = make_shared<Camera>(shared_from_this());
	mCameras.push_back(camera);
	mpRootNode->addChild(static_pointer_cast<Node>(camera));
	return camera;
}