#include "PostToneMap.h"
#include "Mesh.h"
#include "Texture.h"
#include "Fbo.h"
#include "Material.h"
#include "Resource.h"

PostToneMap::PostToneMap()=default;
PostToneMap::~PostToneMap() = default;

void PostToneMap::initPost(int width, int height, std::shared_ptr<Mesh>& pMesh) {
	Post::initPost(width,height,pMesh);
	mpFbo = std::make_unique<Fbo>();
	mpMaterial = Resource::getInstance().cloneMaterial("toneMap");
	assert(pMesh);
	assert(mpFbo);
	assert(mpMaterial);
}

void PostToneMap::process(std::shared_ptr<Texture>& pInput,std::shared_ptr<Texture>& pOutput) {
	if (mpMesh && mpMaterial && mpFbo) {
		mpFbo->replaceColorTexture(pOutput,0);
		mpMaterial->setTextureForSampler("inputTexture", pInput);
		mpMesh->setMaterial(mpMaterial);
		mpFbo->render([this]() {
			mpMesh->draw(nullptr, nullptr);
			});
	}
}