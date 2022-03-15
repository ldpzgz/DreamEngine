#include"Scene.h"
#include "Node.h"
#include "Rect.h"
#include "Camera.h"
#include "Light.h"
#include "Mesh.h"
#include "Material.h"
#include "helper.h"
#include "Resource.h"
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

shared_ptr<Light> Scene::createALight(glm::vec3 pos, glm::vec3 color) {
	auto light = make_shared<Light>();
	mLights.emplace_back(light);
	light->setPosOrDir(pos);
	light->setLightColor(color);
	return light;
}

shared_ptr<Node> Scene::createSkybox() {
	//天空盒
	mSkyboxInfo.mpMesh = make_shared<Mesh>(MeshType::MESH_Cuboid);
	mSkyboxInfo.mpMesh->loadMesh();
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
	pSkyNode->scale(glm::vec3(1000.0f, 1000.0f, 1000.0f));
	return pSkyNode;
}