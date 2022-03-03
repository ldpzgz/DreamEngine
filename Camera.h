#ifndef _CAMERA_H_
#define _CAMERA_H_
#include "Node.h"
class Scene;
template<typename T>
class Vector3;
class Fbo;
class Texture;
class Mesh;
class Camera : virtual public Node<glm::mat4>
{
public:
	Camera(const shared_ptr<Scene>& ps,int w,int h);
	~Camera();
	//参数分别是张开角，宽高比，近平面，远平面
	void perspective(float fovy, float aspect, float n, float f);

	void ortho(float left, float right, float bottom, float top, float zNear, float zFar);

	void renderScene();
	
	//设置宽高比，这个与窗口显示区域不符合的话，画面会变形
	void updateWidthHeight(int w, int h);

	void translate(float x, float y, float z);

	void rotate(float angle, const glm::vec3& vec);

	void lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up);

	Vec3& getPosition() {
		return mPosition;
	}

	std::shared_ptr<Scene> getScene();
private:
	void initDefferedRendering(const std::shared_ptr<Scene>& pScene);
	void renderNode(const shared_ptr<Node<glm::mat4>>& node,
		const std::shared_ptr<Scene>& pScene, std::vector<Vec3>*, std::vector<Vec3>*) const;
	void defferedGeometryPass(const std::shared_ptr<Scene>& pScene) const;
	void defferedLightingPass(std::vector<Vec3>* lightPos,std::vector<Vec3>* lightColor);
	int mWidth;
	int mHeight;
	float fov{ 45.0f };
	float nearp{ 0.1f };
	float farp{ 1000.0f };
	glm::mat4 mProjMatrix{ 1.0f };
	glm::mat4 mViewMatrix{ 1.0f };
	Vec3 mPosition{ 0.0f,0.0f,0.0f };
	weak_ptr<Scene> mpScene;
	std::shared_ptr<Fbo> mpFboDefferedGeo;//用于defered Rendering
	std::shared_ptr<Texture> mpPosMap;
	std::shared_ptr<Texture> mpNormal;
	std::shared_ptr<Texture> mpAlbedoMap;
	std::shared_ptr<Mesh> mpMeshQuad;//for defered rendering lighting pass;
};


#endif