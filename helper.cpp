#include "helper.h"
#include "Material.h"
#include "Fbo.h"
#include "Pbo.h"
#include "Mesh.h"
#include "Resource.h"
#include "Ubo.h"
#include "Sampler.h"
#include <glm/ext/matrix_transform.hpp> //translate, rotate, scale, identity
#include <glm/ext/matrix_clip_space.hpp> // perspective

std::shared_ptr<Texture> genBrdfLut() {
	TextureSP lut = std::make_shared<Texture>();
	auto pSampler = Sampler::getSampler(SamplerType::LinearLinearEdgeEdge);
	lut->setSampler(pSampler);
	lut->create2DMap(512, 512, nullptr, GL_RG16F, GL_RG, GL_FLOAT);

	Mesh mesh(MeshType::Quad);
	mesh.loadMesh();
	mesh.setMaterial(Resource::getInstance().getMaterial("brdfLut"));
	Fbo fbo;
	fbo.attachColorTexture(lut, 0);
	fbo.render([&mesh]() {
		mesh.draw(nullptr, nullptr);
	});
	return lut;
}

TextureSP genSpecularFilterMap(const std::shared_ptr<Texture>& pCube) {
	constexpr int imgWidth = 128;
	//由于opengles3不能渲染到浮点纹理
	TextureSP floatR = std::make_shared<Texture>();
	TextureSP floatG = std::make_shared<Texture>();
	TextureSP floatB = std::make_shared<Texture>();
	TextureSP floatCubeSpec = std::make_shared<Texture>();
	floatCubeSpec->createCubicMap(imgWidth, imgWidth, GL_RGB16F, GL_RGB, GL_FLOAT, true);

	Mesh mesh(MeshType::Cuboid);
	mesh.loadMesh();
	auto& pMaterial = Resource::getInstance().getMaterial("preFilteredEnv");
	pMaterial->setTextureForSampler("skybox", pCube);
	mesh.setMaterial(pMaterial);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};

	unsigned int maxMipLevels = 5;
	for (unsigned int mip = 0; mip < maxMipLevels; ++mip)
	{
		float roughness = (float)mip / (float)(maxMipLevels - 1);
		pMaterial->setRoughness(roughness);
		// reisze framebuffer according to mip-level size.
		unsigned int mipWidth = static_cast<unsigned int>(imgWidth * std::pow(0.5, mip));

		floatR->create2DMap(mipWidth, mipWidth, nullptr, GL_RGBA, GL_RGBA);
		floatG->create2DMap(mipWidth, mipWidth, nullptr, GL_RGBA, GL_RGBA);
		floatB->create2DMap(mipWidth, mipWidth, nullptr, GL_RGBA, GL_RGBA);

		Pbo pbo;
		pbo.initPbo(mipWidth, mipWidth);
		Fbo fbo;
		fbo.attachColorTexture(floatR, 0);
		fbo.attachColorTexture(floatG, 1);
		fbo.attachColorTexture(floatB, 2);

		for (int i = 0; i < 6; ++i) {
			glm::mat4 projView[2]{ captureProjection,captureViews[i] };
			Ubo::getInstance().update("Matrixes", projView, 2 * sizeof(glm::mat4), 0);
			fbo.render([&mesh, &floatCubeSpec, &pbo, mipWidth, i, mip] {
				mesh.draw(nullptr);
				//将渲染得到的三张rgba转换成一张rgb32f
				std::vector<unsigned char> pRGB[3];
				std::vector<float> resultRGB;
				int index = 0;
				std::function<void(Pbo* pbo, void*)> func = [&index, mipWidth, &pRGB](Pbo* pbo, void* pdata) {
					pRGB[index++].assign((unsigned char*)pdata, (unsigned char*)pdata + mipWidth * mipWidth * 4);
				};
				pbo.pullToMem(GL_COLOR_ATTACHMENT0, func);
				pbo.pullToMem(GL_COLOR_ATTACHMENT1, func);
				pbo.pullToMem(GL_COLOR_ATTACHMENT2, func);
				//转换成rgb32f
				resultRGB.resize(3 * mipWidth * mipWidth);
				constexpr float scaleTo = 1000.0f; //shader里面float值先除了1000.0f
				constexpr float scale = 1.0f / 255.0f;
				constexpr float scale2 = 1.0f / 65025.0f;
				constexpr float scale3 = 1.0f / 16581375.0f;
				for (unsigned int i = 0; i < mipWidth; ++i) {
					for (unsigned int j = 0; j < mipWidth; ++j) {
						unsigned int i1 = 4 * (i * mipWidth + j);
						unsigned int r1 = 3 * (i * mipWidth + j);
						resultRGB[r1] = pRGB[0][i1] * scale +
							pRGB[0][i1 + 1] * scale2 +
							pRGB[0][i1 + 2] * scale3 +
							pRGB[0][i1 + 3] * scale * scale3;
						resultRGB[r1 + 1] = pRGB[1][i1] * scale +
							pRGB[1][i1 + 1] * scale2 +
							pRGB[1][i1 + 2] * scale3 +
							pRGB[1][i1 + 3] * scale * scale3;
						resultRGB[r1 + 2] = pRGB[2][i1] * scale +
							pRGB[2][i1 + 1] * scale2 +
							pRGB[2][i1 + 2] * scale3 +
							pRGB[2][i1 + 3] * scale * scale3;
						resultRGB[r1] *= scaleTo;
						resultRGB[r1 + 1] *= scaleTo;
						resultRGB[r1 + 2] *= scaleTo;
					}
				}
				//update到cubemap
				floatCubeSpec->update(0, 0, mipWidth, mipWidth, resultRGB.data(), i, 1, mip);
				});
		}
	}
	return floatCubeSpec;
}

TextureSP genDiffuseIrrMap(const std::shared_ptr<Texture>& pCube) {
	constexpr int irrWidth = 128;
	//由于不能渲染到浮点纹理，用于临时接收irradiance convolution的结果
	auto floatR = std::make_shared<Texture>();
	auto floatG = std::make_shared<Texture>();
	auto floatB = std::make_shared<Texture>();
	//保存irrdiance map到cubemap
	auto floatCubeIrr = std::make_shared<Texture>();

	floatR->create2DMap(irrWidth, irrWidth, nullptr, GL_RGBA, GL_RGBA);
	floatG->create2DMap(irrWidth, irrWidth, nullptr, GL_RGBA, GL_RGBA);
	floatB->create2DMap(irrWidth, irrWidth, nullptr, GL_RGBA, GL_RGBA);
	floatCubeIrr->createCubicMap(irrWidth, irrWidth, GL_RGB16F, GL_RGB, GL_FLOAT);


	Mesh mesh(MeshType::Cuboid);
	mesh.loadMesh();
	auto& pMaterial = Resource::getInstance().getMaterial("irradianceConvolution");
	pMaterial->setTextureForSampler("skybox", pCube);
	mesh.setMaterial(pMaterial);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};
	Pbo pbo;
	pbo.initPbo(irrWidth, irrWidth);
	Fbo fbo;
	fbo.attachColorTexture(floatR, 0);
	fbo.attachColorTexture(floatG, 1);
	fbo.attachColorTexture(floatB, 2);

	for (int i = 0; i < 6; ++i) {
		glm::mat4 projView[2]{ captureProjection,captureViews[i] };
		Ubo::getInstance().update("Matrixes", projView, 2 * sizeof(glm::mat4), 0);
		fbo.render([&mesh, &floatCubeIrr, &pbo, irrWidth, i] {
			mesh.draw(nullptr);
			//将渲染得到的三张rgba转换成一张rgb32f
			std::vector<unsigned char> pRGB[3];
			std::vector<float> resultRGB;
			int index = 0;
			std::function<void(Pbo* pbo, void*)> func = [&index, irrWidth, &pRGB](Pbo* pbo, void* pdata) {
				pRGB[index++].assign((unsigned char*)pdata, (unsigned char*)pdata + irrWidth * irrWidth * 4);
			};
			pbo.pullToMem(GL_COLOR_ATTACHMENT0, func);
			pbo.pullToMem(GL_COLOR_ATTACHMENT1, func);
			pbo.pullToMem(GL_COLOR_ATTACHMENT2, func);
			//转换成rgb32f
			resultRGB.resize(3 * irrWidth * irrWidth);
			constexpr float scaleTo = 1000.0f; //shader里面float值先除了1000.0f
			constexpr float scale = 1.0f / 255.0f;
			constexpr float scale2 = 1.0f / 65025.0f;
			constexpr float scale3 = 1.0f / 16581375.0f;
			for (int i = 0; i < irrWidth; ++i) {
				for (int j = 0; j < irrWidth; ++j) {
					int i1 = 4 * (i * irrWidth + j);
					int r1 = 3 * (i * irrWidth + j);
					resultRGB[r1] = pRGB[0][i1] * scale +
						pRGB[0][i1 + 1] * scale2 +
						pRGB[0][i1 + 2] * scale3 +
						pRGB[0][i1 + 3] * scale * scale3;
					resultRGB[r1 + 1] = pRGB[1][i1] * scale +
						pRGB[1][i1 + 1] * scale2 +
						pRGB[1][i1 + 2] * scale3 +
						pRGB[1][i1 + 3] * scale * scale3;
					resultRGB[r1 + 2] = pRGB[2][i1] * scale +
						pRGB[2][i1 + 1] * scale2 +
						pRGB[2][i1 + 2] * scale3 +
						pRGB[2][i1 + 3] * scale * scale3;
					resultRGB[r1] *= scaleTo;
					resultRGB[r1 + 1] *= scaleTo;
					resultRGB[r1 + 2] *= scaleTo;
				}
			}
			//update到cubemap
			floatCubeIrr->update(0, 0, irrWidth, irrWidth, resultRGB.data(), i);
			});
	}
	//*this = std::move(*floatCubeIrr);
	return floatCubeIrr;
}

TextureSP convertHdrToCubicmap(const std::shared_ptr<Texture>& pHdr) {
	constexpr int width = 1024;
	Fbo fbo;
	//由于不能渲染到浮点纹理，搞三张rgba接收一个浮点的rgb纹理
	TextureSP floatR = std::make_shared<Texture>();
	TextureSP floatG = std::make_shared<Texture>();
	TextureSP floatB = std::make_shared<Texture>();

	//保存hdr到cubemap
	TextureSP floatCube = std::make_shared<Texture>();

	Mesh mesh(MeshType::Cuboid);
	mesh.loadMesh();
	auto& pMaterial = Resource::getInstance().getMaterial("hdrToCubicMap");
	pMaterial->setTextureForSampler("equirectangularMap", pHdr);
	mesh.setMaterial(pMaterial);
	auto pSampler = Sampler::getSampler(SamplerType::LinearLinearEdgeEdge);
	floatR->create2DMap(width, width, nullptr, GL_RGBA, GL_RGBA);
	floatG->create2DMap(width, width, nullptr, GL_RGBA, GL_RGBA);
	floatB->create2DMap(width, width, nullptr, GL_RGBA, GL_RGBA);
	floatCube->createCubicMap(width, width, GL_RGB16F, GL_RGB, GL_FLOAT);
	fbo.attachColorTexture(floatR, 0);
	fbo.attachColorTexture(floatG, 1);
	fbo.attachColorTexture(floatB, 2);

	glm::mat4 captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
	glm::mat4 captureViews[] =
	{
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f,  0.0f,  0.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  1.0f,  0.0f), glm::vec3(0.0f,  0.0f,  1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f,  0.0f), glm::vec3(0.0f,  0.0f, -1.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f,  1.0f), glm::vec3(0.0f, -1.0f,  0.0f)),
		glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f,  0.0f, -1.0f), glm::vec3(0.0f, -1.0f,  0.0f))
	};
	Pbo pbo;
	pbo.initPbo(width, width);
	for (int i = 0; i < 6; ++i) {
		glm::mat4 projView[2]{ captureProjection,captureViews[i]};
		Ubo::getInstance().update("Matrixes", projView, 2 * sizeof(glm::mat4), 0);
		fbo.render([&mesh, &floatCube, &pbo, width, i] {
			mesh.draw(nullptr);
			//将渲染得到的三张rgba转换成一张rgb32f
			std::vector<unsigned char> pRGB[3];
			std::vector<float> resultRGB;
			int index = 0;
			std::function<void(Pbo* pbo, void*)> func = [&index, width, &pRGB](Pbo* pbo, void* pdata) {
				pRGB[index++].assign((unsigned char*)pdata, (unsigned char*)pdata + width * width * 4);
			};
			pbo.pullToMem(GL_COLOR_ATTACHMENT0, func);
			pbo.pullToMem(GL_COLOR_ATTACHMENT1, func);
			pbo.pullToMem(GL_COLOR_ATTACHMENT2, func);
			//转换成rgb32f
			resultRGB.resize(3 * width * width);
			constexpr float scaleTo = 100.0f; //shader里面float值先除了100.0f
			constexpr float scale = 1.0f / 255.0f;
			constexpr float scale2 = 1.0f / 65025.0f;
			constexpr float scale3 = 1.0f / 16581375.0f;
			for (int i = 0; i < width; ++i) {
				for (int j = 0; j < width; ++j) {
					int i1 = 4 * (i * width + j);
					int r1 = 3 * (i * width + j);
					resultRGB[r1] = pRGB[0][i1] * scale +
						pRGB[0][i1 + 1] * scale2 +
						pRGB[0][i1 + 2] * scale3 +
						pRGB[0][i1 + 3] * scale * scale3;
					resultRGB[r1 + 1] = pRGB[1][i1] * scale +
						pRGB[1][i1 + 1] * scale2 +
						pRGB[1][i1 + 2] * scale3 +
						pRGB[1][i1 + 3] * scale * scale3;
					resultRGB[r1 + 2] = pRGB[2][i1] * scale +
						pRGB[2][i1 + 1] * scale2 +
						pRGB[2][i1 + 2] * scale3 +
						pRGB[2][i1 + 3] * scale * scale3;
					resultRGB[r1] *= scaleTo;
					resultRGB[r1 + 1] *= scaleTo;
					resultRGB[r1 + 2] *= scaleTo;
				}
			}
			//update到cubemap
			floatCube->update(0, 0, width, width, resultRGB.data(), i);
			});
	}
	return floatCube;
}

TextureSP genSSAO() {
	return nullptr;
}