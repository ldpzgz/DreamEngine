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
class Material;
class Shader;
class Post;
/*
* 这个camera可以监听一个节点，节点方位变化的时候会通知相机，camera根据情况更新自己的相机。
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
	void initDefferedRendering(const std::shared_ptr<Scene>& pScene) noexcept;
	//如果物体不共用同一个shader，使用这个renderNode
	void renderNode(const std::shared_ptr<Node>& node,
		const std::shared_ptr<Scene>& pScene, 
		std::vector<glm::vec3>*, 
		std::vector<glm::vec3>*) const noexcept;

	//如果所有物体共用同一个shader，使用这个renderNode
	void renderNode(const std::shared_ptr<Node>& node, 
		int posLoc, int texcoordLoc, int normalLoc,
		std::shared_ptr<Shader>& pShader) const noexcept;

	void defferedGeometryPass(const std::shared_ptr<Scene>& pScene) const noexcept;

	void genShadowMap(std::shared_ptr<Scene>& pScene);
	
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

	//shadow map
	std::unique_ptr<Fbo> mpFboShadowMap;
	std::shared_ptr<Texture> mpShadowMap;
	std::shared_ptr<Material> mpGenShadowMaterial;
	
	//for deffered rendering
	std::shared_ptr<Texture> mpPosMap;
	std::shared_ptr<Texture> mpNormal;
	std::shared_ptr<Texture> mpAlbedoMap;
	std::shared_ptr<Texture> mpDepthMap;

	//deffered rendering
	std::shared_ptr<Fbo> mpFboDefferedGeo;//用于defered geometry pass
	std::shared_ptr<Fbo> mpFboDefferedLighting;//用于defered lighting pass
	std::shared_ptr<Fbo> mpFboForward;//用于forward render
	std::shared_ptr<Texture> mpRenderResult;
	std::shared_ptr<Material> mpDefLightPassMaterial;

	//ssao
	std::shared_ptr<Fbo> mpFboSsao;
	std::shared_ptr<Fbo> mpFboSsaoBlured;
	std::shared_ptr<Material> mpSsaoMaterial;
	std::shared_ptr<Material> mpSsaoBlurMaterial;
	std::shared_ptr<Texture> mpSsaoNoiseMap;
	std::shared_ptr<Texture> mpSsaoBluredMap;
	std::shared_ptr<Texture> mpSsaoResultMap;
	std::vector<glm::vec4> mSsaoKernel;

	//Temporal Anti Aliasing
	std::shared_ptr<Fbo> mpFboTaa;
	std::shared_ptr<Material> mpTaaMaterial;//for defered rendering lighting pass;
	std::shared_ptr<Texture> mpTaaVelocityMap;
	std::shared_ptr<Texture> mpTaaPreColorMap[2];
	int mTaaPreColorMapIndex{ 0 };
	int mTaaFrameCount{0};
	int mTaaOffsetIndex{ 0 };

	std::shared_ptr<Mesh> mpPostMesh;
	std::shared_ptr<Material> mpDrawQuadMaterial;

	std::shared_ptr<Texture> mpPostTex[2];
	std::unique_ptr<Post> mpPostTonemap;
	std::unique_ptr<Post> mpPostSmaa;
};


#endif