#ifndef _SCENE_H_
#define _SCENE_H_
#include "Node.h"
#include <memory>
#include <vector>
using namespace std;
class Camera;
class Light;
class Scene : public enable_shared_from_this<Scene> {
public:
	Scene();
	~Scene();
	shared_ptr<Node<glm::mat4>>& getRoot() noexcept{
		return mpRootNode;
	}

	shared_ptr<Camera> createACamera();

	shared_ptr<Light> createALight();
private:
	shared_ptr<Node<glm::mat4>> mpRootNode;
	vector<shared_ptr<Camera>> mCameras;
	vector<shared_ptr<Light>> mLights;
};


#endif