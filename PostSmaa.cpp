#include "PostSmaa.h"
#include "Mesh.h"
#include "Material.h"
#include "Texture.h"
#include "Resource.h"
#include "AreaTex.h"
#include "SearchTex.h"
#include "Fbo.h"
#include "Log.h"

PostSmaa::PostSmaa() = default;
PostSmaa::~PostSmaa()=default;

void PostSmaa::initPost(int width,int height,std::shared_ptr<Mesh>& pMesh) {
	Post::initPost(width,height,pMesh);
	assert(pMesh);
	mpEdgeTex = std::make_shared<Texture>();
	mpWeightTex = std::make_shared<Texture>();
	mpAreaTex = std::make_shared<Texture>();
	mpSearchTex = std::make_shared<Texture>();
	assert(mpWeightTex);
	assert(mpAreaTex);
	assert(mpSearchTex);
	
	mpWeightTex->create2DMap(width, height, nullptr,GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
	mpEdgeTex->create2DMap(width, height, nullptr, GL_RG8, GL_RG, GL_UNSIGNED_BYTE);
	auto temp = std::make_unique<unsigned char[]>(AREATEX_PITCH * AREATEX_HEIGHT);
	for (int i = 0; i < AREATEX_HEIGHT; ++i) {
		std::copy(areaTexBytes + (AREATEX_HEIGHT - 1 - i) * AREATEX_PITCH,
			areaTexBytes + (AREATEX_HEIGHT - i) * AREATEX_PITCH, temp.get() + i * AREATEX_PITCH);
	}
	mpAreaTex->create2DMap(AREATEX_WIDTH, AREATEX_HEIGHT, temp.get(), GL_RG8, GL_RG, GL_UNSIGNED_BYTE);
	temp = std::make_unique<unsigned char[]>(SEARCHTEX_PITCH * SEARCHTEX_HEIGHT);
	for (int i = 0; i < SEARCHTEX_HEIGHT; ++i) {
		std::copy(searchTexBytes + (SEARCHTEX_HEIGHT - 1 - i) * SEARCHTEX_PITCH,
			searchTexBytes + (SEARCHTEX_HEIGHT - i) * SEARCHTEX_PITCH, temp.get() + i * SEARCHTEX_PITCH);
	}
	mpSearchTex->create2DMap(SEARCHTEX_WIDTH, SEARCHTEX_HEIGHT, temp.get(), GL_R8, GL_RED, GL_UNSIGNED_BYTE);
	
	Resource& res = Resource::getInstance();
	mpEdgeMaterial = res.getMaterial("smaaEdge");
	mpWeightsMaterial = res.getMaterial("smaaWeights");
	mpBlendMaterial = res.getMaterial("smaaBlend");
	assert(mpEdgeMaterial);
	assert(mpWeightsMaterial);
	assert(mpBlendMaterial);
	mpWeightsMaterial->setTextureForSampler("areaTex",mpAreaTex);
	mpWeightsMaterial->setTextureForSampler("searchTex",mpSearchTex);

	mpFboDetectEdge = std::make_unique<Fbo>();
	mpFboWeights = std::make_unique<Fbo>();
	mpFboBlend = std::make_unique<Fbo>();

	mpFboDetectEdge->attachColorTexture(mpEdgeTex, 0);
	mpFboWeights->attachColorTexture(mpWeightTex, 0);
}

void PostSmaa::process(std::shared_ptr<Texture>& pInput,
	std::shared_ptr<Texture>& pOutput) {
	//±ßÔµ¼ì²â
	mpEdgeMaterial->setTextureForSampler("colorTex", pInput);
	mpMesh->setMaterial(mpEdgeMaterial);
	mpFboDetectEdge->render([this]() {
		mpMesh->draw(nullptr, nullptr);
		});

	//¼ÆËãÈ¨ÖØ
	mpWeightsMaterial->setTextureForSampler("edgesTex", mpEdgeTex);
	mpMesh->setMaterial(mpWeightsMaterial);
	mpFboWeights->render([this]() {
		mpMesh->draw(nullptr, nullptr);
		});

	//blend
	mpFboBlend->replaceColorTexture(pOutput, 0);
	mpBlendMaterial->setTextureForSampler("colorTex",pInput);
	mpBlendMaterial->setTextureForSampler("blendTex", mpWeightTex);
	mpMesh->setMaterial(mpBlendMaterial);
	mpFboBlend->render([this]() {
		mpMesh->draw(nullptr, nullptr);
		});
}
