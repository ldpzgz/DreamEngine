#ifndef _CAMERA_H_
#define _CAMERA_H_
#include <memory>
#include <vector>
#include "NodeListener.h"
class Scene;
class Fbo;
class Texture;
class Mesh;
class Renderable;
class Node;
/*
* 这个camera是一个节点，相当于节点上绑了一个相机，节点的方位就是相机的方位。
* 但是camera节点，以及其所有父节点都只能做旋转和平移变换
*/
class Camera : public NodeListener
{
public:
	Camera(const std::shared_ptr<Scene>& ps,int w,int h);
	~Camera();
	//参数分别是张开角，宽高比，近平面，远平面
	void perspective(float fovy, float aspect, float n, float f);

	void ortho(float left, float right, float bottom, float top, float zNear, float zFar);

	void renderScene();
	
	//设置宽高比，这个与窗口显示区域不符合的话，画面会变形
	void updateWidthHeight(int w, int h);

	glm::vec3& getPosition() {
		return mPosition;
	}

	void update(const glm::mat4& mat) override;
	/*
	* 根据eyepos,center,up构造一个mViewMatrix,用于转换场景中的其他物体
	* 注意eyepos，center，up都是本地坐标系
	*/
	void lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up) noexcept;

	std::shared_ptr<Scene> getScene();

	const glm::mat4& getViewMatrix() {
		return mViewMat;
	}
private:
	void initDefferedRendering(const std::shared_ptr<Scene>& pScene);
	void renderNode(const std::shared_ptr<Node>& node,
		const std::shared_ptr<Scene>& pScene, std::vector<glm::vec3>*, std::vector<glm::vec3>*) const;
	void defferedGeometryPass(const std::shared_ptr<Scene>& pScene) const;
	void defferedLightingPass(std::vector<glm::vec3>* lightPos,std::vector<glm::vec3>* lightColor);
	int mWidth;
	int mHeight;
	float fov{ 45.0f };
	float nearp{ 0.1f };
	float farp{ 1000.0f };
	glm::mat4 mProjMatrix{ 1.0f };
	glm::mat4 mWorldMat{ 1.0f };
	glm::mat4 mViewMat{ 1.0f };
	glm::vec3 mUp{ 0.0f,1.0f,0.0f };
	glm::vec3 mLookAt{ 0.0f,0.0f,0.0f };
	glm::vec3 mPosition{ 0.0f,0.0f,1.0f };
	std::weak_ptr<Scene> mpScene;
	std::shared_ptr<Fbo> mpFboDefferedGeo;//用于defered Rendering
	std::shared_ptr<Texture> mpPosMap;
	std::shared_ptr<Texture> mpNormal;
	std::shared_ptr<Texture> mpAlbedoMap;
	std::shared_ptr<Mesh> mpMeshQuad;//for defered rendering lighting pass;
};


#endif