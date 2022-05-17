#include "Light.h"
#include"Scene.h"
#include "Node.h"
#include "Rect.h"
#include "Camera.h"
#include "Mesh.h"
#include "Material.h"
#include "helper.h"
#include "Resource.h"
#include "Ubo.h"
#include "Log.h"



Scene::Scene() {
	mpRootNode = make_shared<Node>();
	mpRootNodeDeffered = make_shared<Node>();
}
Scene::~Scene() {

}

shared_ptr<Camera> Scene::createACamera(int w,int h) {
	auto camera = make_shared<Camera>(shared_from_this(),w,h);
	mCameras.push_back(camera);
	mpRootNode->addListener(dynamic_pointer_cast<NodeListener>(camera));
	return camera;
}

shared_ptr<Light> Scene::createALight(glm::vec3 pos, glm::vec3 color, LightType type) {
	if (mLights.size() >= MaxNumberOfLights) {
		LOGE("the max number of lights in a scene is 15");
		return nullptr;
	}
	auto light = make_shared<Light>(type);
	mLights.emplace_back(light);
	light->setPosOrDir(pos);
	light->setLightColor(color);
	return light;
}

shared_ptr<Node> Scene::createSkybox() {
	//天空盒
	mSkyboxInfo.mpMesh = make_shared<Mesh>(MeshType::Cuboid);
	mSkyboxInfo.mpMesh->loadMesh();
	mSkyboxInfo.mpMesh->setCastShadow(false);
	mSkyboxInfo.mpMesh->setReceiveShadow(false);
	auto& pSkyboxMaterial = Resource::getInstance().getMaterial("skyboxHdr1");
	auto& pHdrTex = pSkyboxMaterial->getTextureOfSampler("skybox");
	if (pHdrTex) {
		auto pCube = convertHdrToCubicmap(pHdrTex);
		mSkyboxInfo.mpIrrTex = genDiffuseIrrMap(pCube);
		mSkyboxInfo.mpPrefilterTex = genSpecularFilterMap(pCube);
		pSkyboxMaterial->setTextureForSampler("skybox", pCube);
	}
	//Material::getTexture("hdr/memorial.hdr")->convertHdrToCubicmap();
	mSkyboxInfo.mpMesh->setMaterial(pSkyboxMaterial);
	auto pSkyNode = mpRootNode->newAChild();
	pSkyNode->addRenderable(mSkyboxInfo.mpMesh);
	pSkyNode->scale(glm::vec3(3000.0f, 3000.0f, 3000.0f));
	return pSkyNode;
}

void Scene::updateLightsForShader(const glm::mat4& viewMat) {
	//获取场景中的灯光
	std::vector<glm::vec4> lightPos;
	std::vector<glm::vec4> lightColor;
	
	for (const auto& pl : mLights) {
		if (pl && pl->isPointLight()) {
			auto& pos = pl->getPosOrDir();
			auto tpos = viewMat * glm::vec4(pos.x, pos.y, pos.z, 1.0f);
			lightPos.emplace_back(tpos);
			lightColor.emplace_back(glm::vec4(pl->getLightColor(), 1.0f));
		}
	}
	if (!lightPos.empty()) {
		int counts = static_cast<int>(lightPos.size());
		int byteSize = counts * sizeof(glm::vec4);
		constexpr int colorStartPos = MaxNumberOfLights * sizeof(glm::vec4);
		constexpr int countStartPos = MaxNumberOfLights * sizeof(glm::vec4) * 2;
		Ubo::getInstance().update("Lights", lightPos.data(), byteSize,0);
		Ubo::getInstance().update("Lights", lightColor.data(), byteSize, colorStartPos);
		Ubo::getInstance().update("Lights", &counts, 4, countStartPos);
	}

}