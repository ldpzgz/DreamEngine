#ifndef _CAMERA_H_
#define _CAMERA_H_
#include "Node.h"
class Scene;
class Camera : virtual public Node
{
public:
	explicit Camera(shared_ptr<Scene> ps);
	Camera(shared_ptr<Scene> ps,float asp);
	~Camera();
	//参数分别是张开角，宽高比，近平面，远平面
	void perspective(float fovy, float aspect, float n, float f);

	void renderScene();

	//设置宽高比，这个与窗口显示区域不符合的话，画面会变形
	void setAspect(float asp);
private:
	float aspect;//屏幕宽高比
	float fov;
	float nearp;
	float farp;
	glm::mat4 mProj;
	weak_ptr<Scene> mpScene;
};


#endif