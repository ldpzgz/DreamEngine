#include "PostGsBlur.h"
#include "Mesh.h"
#include "Texture.h"
#include "Fbo.h"
#include "Material.h"
#include "Resource.h"
#include "Shader.h"
#include <algorithm>

PostGsBlur::PostGsBlur()=default;
PostGsBlur::~PostGsBlur() = default;

void PostGsBlur::initPost(int width, int height, std::shared_ptr<Mesh>& pMesh) {
	Post::initPost(width,height,pMesh);
	mpFbo = std::make_unique<Fbo>();
	mpMaterial = Resource::getInstance().cloneMaterial("gaussianBlur");
	assert(pMesh);
	assert(mpFbo);
	assert(mpMaterial);
	
	auto gsFunc = [](float x, float y)->float {
		constexpr float sigma = 2.6f;
		constexpr float sigma2 = sigma * sigma;
		constexpr float sigma2_2 = 2.0f * sigma2;
		constexpr float pi = 3.1415926f;
		constexpr float pi2 = 2.0f * pi;
		constexpr float e = 2.7182804f;
		constexpr float invPiSigma = 1.0f / (pi2 * sigma2);
		return invPiSigma * pow(e, -(x * x + y * y) / sigma2_2);
	};
	float totalW = 0.0f;
	for (int i = -2; i <= 2; ++i) {
		mOffset[ i + 2] = i * 1.0f / width;
		float temp = gsFunc(i, 0);
		mWeights[i + 2] = temp;
		totalW += temp;
		//mOffset[5+i+2] = i * 1.0f / height;
		/*for (int j = -2; j <= 2; ++j) {
			float temp = gsFunc(i, j);
			mWeights[(i + 2)*5 + j + 2] = temp;
			totalW += temp;
		}*/
	}
	for_each(mWeights.begin(), mWeights.end(),[totalW](float& p) { p *= 1.0f / totalW; });
	
}

void PostGsBlur::process(std::shared_ptr<Texture>& pInput,std::shared_ptr<Texture>& pOutput) {
	if (mpMesh && mpMaterial && mpFbo) {
		mpFbo->replaceColorTexture(pOutput,0);
		checkglerror();
		mpMaterial->setTextureForSampler("texForFilt", pInput);
		mpMesh->setMaterial(mpMaterial);
		mpFbo->render([this]() {
			float vertical = 1.0f;
			if (isVertical) {
				vertical = 1.0f;
				mpMaterial->setUniform1fv("isVertical", 1, &vertical);
			}
			else {
				vertical = 0.0f;
				mpMaterial->setUniform1fv("isVertical", 1, &vertical);
			}
			isVertical = !isVertical;
			mpMaterial->setUniform1fv("w", mWeights.size(), mWeights.data());
			mpMaterial->setUniform1fv("offset", mOffset.size(), mOffset.data());
			mpMesh->draw(nullptr, nullptr);
			});
	}
}