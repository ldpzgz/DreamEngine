#ifndef _SCENE_H_
#define _SCENE_H_
#include "Node.h"
#include <memory>
#include <vector>
using namespace std;
class Camera;
class Scene : enable_shared_from_this<Scene> {
public:
	Scene();
	~Scene();
	shared_ptr<Node>& getRoot();

	shared_ptr<Camera> createACamera();
private:
	shared_ptr<Node> mpRootNode;
	vector<shared_ptr<Camera>> mCameras;
};


#endif