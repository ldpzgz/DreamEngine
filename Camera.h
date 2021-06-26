#ifndef _CAMERA_H_
#define _CAMERA_H_
#include "Node.h"
class Scene;
class Camera : virtual public Node
{
public:
	explicit Camera(shared_ptr<Scene> ps);
	~Camera();
	//参数分别是张开角，宽高比，近平面，远平面
	void perspective(float fovy, float aspect, float n, float f);

	void renderScene();
private:
	glm::mat4 mProj;
	shared_ptr<Scene> mpScene;
};


#endif