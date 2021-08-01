#ifndef _CAMERA_H_
#define _CAMERA_H_
#include "Node.h"
class Scene;
class Camera : virtual public Node
{
public:
	explicit Camera(const shared_ptr<Scene>& ps);
	Camera(const shared_ptr<Scene>& ps,float asp);
	~Camera();
	//参数分别是张开角，宽高比，近平面，远平面
	void perspective(float fovy, float aspect, float n, float f);

	void ortho(float left, float right, float bottom, float top, float zNear, float zFar);

	void renderScene();

	void renderNode(const shared_ptr<Node>& node) const;
	//设置宽高比，这个与窗口显示区域不符合的话，画面会变形
	void setAspect(float asp);

	void translate(float x, float y, float z) override;

	void lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up) override;
private:
	float aspect;//屏幕宽高比
	float fov;
	float nearp;
	float farp;
	glm::mat4 mProjMatrix;
	glm::mat4 mProjViewMatrix;
	weak_ptr<Scene> mpScene;
};


#endif