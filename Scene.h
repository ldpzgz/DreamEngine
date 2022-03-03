#ifndef _SCENE_H_
#define _SCENE_H_
#include "Node.h"
#include <memory>
#include <vector>
using namespace std;
class Camera;
class Light;
class Mesh;
class Texture;
template<typename T>
class Vector3;
//场景里面有相机，灯光，天空盒
class Scene : public enable_shared_from_this<Scene> {
public:
	class SkyboxInfo {
	public:
		std::shared_ptr<Mesh> mpMesh;
		std::shared_ptr<Texture> mpHdrTex;
		std::shared_ptr<Texture> mpIrrTex;//irradiance map
		std::shared_ptr<Texture> mpPrefilterTex;//specular map
	};
	Scene();
	~Scene();
	shared_ptr<Node<glm::mat4>>& getRoot() noexcept{
		return mpRootNode;
	}

	shared_ptr<Node<glm::mat4>>& getRootDeffered() noexcept {
		return mpRootNodeDeffered;
	}

	shared_ptr<Camera> createACamera(int w,int h);

	shared_ptr<Light> createALight(Vec3 pos,Vec3 color);

	bool createSkybox();

	const SkyboxInfo& getSkybox() const{
		return mSkyboxInfo;
	}

	const vector<shared_ptr<Light>>& getLights() const {
		return mLights;
	}
private:
	shared_ptr<Node<glm::mat4>> mpRootNode;
	shared_ptr<Node<glm::mat4>> mpRootNodeDeffered;//走deffered rendering
	vector<shared_ptr<Camera>> mCameras;
	vector<shared_ptr<Light>> mLights;
	SkyboxInfo mSkyboxInfo;
};


#endif