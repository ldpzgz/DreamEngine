#ifndef _SCENE_H_
#define _SCENE_H_
#include <memory>
#include <vector>
#include <glm/vec3.hpp>           // vec3
using namespace std;
class Node;
class Camera;
class Light;
class Mesh;
class Texture;
enum class LightType;
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
	shared_ptr<Node>& getRoot() noexcept{
		return mpRootNode;
	}

	shared_ptr<Node>& getRootDeffered() noexcept {
		return mpRootNodeDeffered;
	}

	shared_ptr<Camera> createACamera(int w,int h);

	shared_ptr<Light> createALight(glm::vec3 pos, glm::vec3 color, LightType type=LightType::Point);

	shared_ptr<Node> createSkybox();

	const SkyboxInfo& getSkybox() const{
		return mSkyboxInfo;
	}

	const vector<shared_ptr<Light>>& getLights() const {
		return mLights;
	}
private:
	shared_ptr<Node> mpRootNode;
	shared_ptr<Node> mpRootNodeDeffered;//走deffered rendering
	vector<shared_ptr<Camera>> mCameras;
	vector<shared_ptr<Light>> mLights;
	SkyboxInfo mSkyboxInfo;
};


#endif