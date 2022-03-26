#include "Node.h"
#include "Rect.h"
#include "Camera.h"
#include "Mesh.h"
#include "Scene.h"
#include "Light.h"
#include "Shader.h"
#include "Fbo.h"
#include "Log.h"
#include "Resource.h"
#include <glm/ext/matrix_transform.hpp>
#include <random>

constexpr glm::vec2 Halton23[8] =
{
	glm::vec2(0.0f, -1.0f / 3.0f),
	glm::vec2(-1.0f / 2.0f, 1.0f / 3.0f),
	glm::vec2(1.0f / 2.0f, -7.0f / 9.0f),
	glm::vec2(-3.0f / 4.0f, -1.0f / 9.0f),
	glm::vec2(1.0f / 4.0f, 5.0f / 9.0f),
	glm::vec2(-1.0f / 4.0f, -5.0f / 9.0f),
	glm::vec2(3.0f / 4.0f, 1.0f / 9.0f),
	glm::vec2(-7.0f / 8.0f, 7.0f / 9.0f)
};

Camera::Camera(const shared_ptr<Scene>& ps, int w,int h) :
	mWidth(w),
	mHeight(h),
	mpScene(ps)
{
	perspective(fov, static_cast<float>(w)/ static_cast<float>(h),nearp,farp);
}

Camera::~Camera()
{
}

void Camera::perspective(float fovy, float asp, float n, float f) {
	mProjMatrix = glm::perspective(fovy, asp,n,f);
	fov = fovy;
	nearp = n;
	farp = f;
}

void Camera::ortho(float left, float right, float bottom, float top, float zNear, float zFar) {
	mProjMatrix = glm::ortho(left,right,bottom,top,zNear,zFar);
	mPosition.x = (left + right) / 2.0f;
	mPosition.y = (bottom + top) / 2.0f;
	mPosition.z = zNear+1.0f;
}
//只有旋转与平移的矩阵求逆
glm::mat4 inverseLookAt(glm::mat4& mat) {
	glm::mat4 temp{ 1.0f };
	temp[0][0] = mat[0][0];
	temp[1][0] = mat[0][1];
	temp[2][0] = mat[0][2];
	temp[0][1] = mat[1][0];
	temp[1][1] = mat[1][1];
	temp[2][1] = mat[1][2];
	temp[0][2] = mat[2][0];
	temp[1][2] = mat[2][1];
	temp[2][2] = mat[2][2];
	temp[0][3] = -mat[0][3] * temp[0][0] - mat[1][3] * temp[0][1] - mat[2][3] * temp[0][2];
	temp[1][3] = -mat[0][3] * temp[1][0] - mat[1][3] * temp[1][1] - mat[2][3] * temp[1][2];
	temp[2][3] = -mat[0][3] * temp[2][0] - mat[1][3] * temp[2][1] - mat[2][3] * temp[2][2];
	return temp;
}

void Camera::update(const glm::mat4& mat) {
	mWorldMat = mat;
	glm::vec3 pos(mWorldMat * glm::vec4(mPosition, 1.0f));
	glm::vec3 lookat(mWorldMat * glm::vec4(mLookAt, 1.0f));
	mViewMat = glm::lookAt(pos, lookat, mUp);
}

void Camera::lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up) noexcept {
	mPosition = eyepos; 
	mLookAt = center;
	mUp = up;
	glm::vec3 pos(mWorldMat* glm::vec4(eyepos, 1.0f));
	glm::vec3 lookat(mWorldMat * glm::vec4(center, 1.0f));
	mViewMat = glm::lookAt(pos, lookat, mUp);
}

void Camera::renderScene() {
	auto pScene = mpScene.lock();
	if (pScene) {
		//获取场景中的灯光
		std::vector<glm::vec3> lightPos;
		std::vector<glm::vec3> lightColor;
		const auto& lights = pScene->getLights();
		for (const auto& pl : lights) {
			if (pl) {
				auto& pos = pl->getPosOrDir();
				auto tpos = mViewMat * glm::vec4(pos.x, pos.y, pos.z,1.0f);
				lightPos.emplace_back(glm::vec3(tpos.x,tpos.y,tpos.z));
				lightColor.emplace_back(pl->getLightColor());
			}
		}
		
		//deffered rendering
		if (!mpFboDefferedGeo) {
			initDefferedRendering(pScene);
		}
		if (mpFboDefferedGeo) {
			float temp[2];
			int taaUniform[2];
			temp[0] = mWidth;
			temp[1] = mHeight;
			taaUniform[0] = mTaaFrameCount++;
			taaUniform[1] = mTaaOffsetIndex++;
			mTaaOffsetIndex %= 8;
			if (mTaaFrameCount > 100000000) {
				//避免长时间运行溢出
				mTaaFrameCount = 1;
			}
			Ubo::getInstance().update("ScreenWH", temp, sizeof(temp));
			Ubo::getInstance().update("Taa", taaUniform, sizeof(taaUniform));

			mpFboDefferedGeo->render([this, &pScene] {
				//deffered rendering geometry pass
				defferedGeometryPass(pScene);
				});
			//ssao
			if (mpFboSsao) {
				mpFboSsao->render([this] {
					mpMeshQuad->setMaterial(mpSsaoMaterial);
					mpMeshQuad->draw(&mProjMatrix,nullptr);
					});
			}
			//ssao blur
			if (mpFboSsaoBlured) {
				mpFboSsaoBlured->render([this] {
					mpMeshQuad->setMaterial(mpSsaoBlurMaterial);
					mpMeshQuad->draw(nullptr, nullptr);
					});
			}

			//deffered render lighting pass
			if (mpFboDefferedLighting) {
				mpFboDefferedLighting->render([this, &lightPos, &lightColor]() {
					mpMeshQuad->setMaterial(mpDefLightPassMaterial);
					mpMeshQuad->draw(nullptr, nullptr, nullptr, nullptr, &lightPos, &lightColor);
					});
			}
			//post-process
			//taa
			if (mpTaaMaterial) {
				//previousColor map is set dynamicly
				mpTaaMaterial->setTextureForSampler("previousColor", mpTaaPreColorMap[mTaaPreColorMapIndex]);
				mTaaPreColorMapIndex = (mTaaPreColorMapIndex + 1) % 2;
				mpFboTaa->replaceColorTexture(mpTaaPreColorMap[mTaaPreColorMapIndex], 0);
				mpMeshQuad->setMaterial(mpTaaMaterial);
				mpFboTaa->render([this]() {
					mpMeshQuad->draw(nullptr, nullptr);
				});

				if (mpDrawQuadMaterial) {
					mpDrawQuadMaterial->setTextureForSampler("albedoMap", mpTaaPreColorMap[mTaaPreColorMapIndex]);
				}
			}
			mpMeshQuad->setMaterial(mpDrawQuadMaterial);
			mpMeshQuad->draw(nullptr, nullptr);
			//把深度缓冲区copy到窗口系统
			mpFboDefferedGeo->blitDepthBufToWin();
			
		}
		//forward rendering
		const auto& rootNode = pScene->getRoot();
		renderNode(rootNode, pScene, &lightPos, &lightColor);
	}
}

void Camera::renderNode(const shared_ptr<Node>& node, 
	const std::shared_ptr<Scene>& pScene,
	std::vector<glm::vec3>* lightPos, 
	std::vector<glm::vec3>* lightColor) const
{
	if (node) {
		const auto& pRenderables = node->getRenderables();
		glm::mat4 modelMat = node->getWorldMatrix();

		for (const auto& pRen : pRenderables) {
			//std::shared_ptr<R> pMesh = std::dynamic_pointer_cast<Mesh>(pRen.second);
			if (pRen.second) {
				pRen.second->draw(&mProjMatrix, &modelMat, &mViewMat, nullptr,lightPos, lightColor, &mPosition);
			}
		}
		
		const auto& pChildNodes = node->getChildren();
		for (const auto& pNode : pChildNodes) {
			renderNode(pNode.second,pScene, lightPos, lightColor);
		}
	}
}

void Camera::initDefferedRendering(const std::shared_ptr<Scene>& pScene) {
	mpFboDefferedGeo = make_shared<Fbo>();
	mpFboDefferedLighting = make_shared<Fbo>();
	mpFboSsao = make_shared<Fbo>();
	mpFboSsaoBlured = make_shared<Fbo>();
	mpFboTaa = make_shared<Fbo>();
	mpPosMap = std::make_shared<Texture>();//0
	mpNormal = std::make_shared<Texture>();//1
	mpAlbedoMap = std::make_shared<Texture>();//2
	mpDepthMap = std::make_shared<Texture>();
	mpDefferedRenderResult = std::make_shared <Texture>();
	mpSsaoResultMap = std::make_shared<Texture>();
	mpSsaoNoiseMap = std::make_shared<Texture>();
	mpSsaoBluredMap = std::make_shared<Texture>();
	mpTaaVelocityMap = std::make_shared<Texture>();
	mpTaaPreColorMap[0] = std::make_shared<Texture>();
	mpTaaPreColorMap[1] = std::make_shared<Texture>();
	if (mpPosMap && mpNormal && mpAlbedoMap && mpDepthMap && mpTaaVelocityMap) {
		mpPosMap->setParam(GL_NEAREST, GL_NEAREST);
		mpPosMap->create2DMap(mWidth, mHeight, nullptr, GL_RGBA16F, GL_RGBA, GL_FLOAT);
		mpNormal->setParam(GL_NEAREST, GL_NEAREST);
		mpNormal->create2DMap(mWidth, mHeight, nullptr, GL_RGBA16F, GL_RGBA, GL_FLOAT);
		mpAlbedoMap->create2DMap(mWidth, mHeight, nullptr, GL_RGBA16F, GL_RGBA, GL_FLOAT);
		mpDepthMap->setParam(GL_NEAREST, GL_NEAREST);
		mpDepthMap->create2DMap(mWidth, mHeight, nullptr, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
		mpTaaVelocityMap->setParam(GL_NEAREST, GL_NEAREST);
		mpTaaVelocityMap->create2DMap(mWidth, mHeight, nullptr, GL_RG16F, GL_RG, GL_FLOAT);
		mpFboDefferedGeo->attachColorTexture(mpPosMap, 0);
		mpFboDefferedGeo->attachColorTexture(mpNormal, 1);
		mpFboDefferedGeo->attachColorTexture(mpAlbedoMap, 2);
		mpFboDefferedGeo->attachColorTexture(mpTaaVelocityMap, 3);
		mpFboDefferedGeo->attachDepthTexture(mpDepthMap);//深度缓存
	}
	if (mpDefferedRenderResult) {
		mpDefferedRenderResult->create2DMap(mWidth,mHeight,nullptr, GL_RGBA16F, GL_RGBA,GL_FLOAT);
		mpFboDefferedLighting->attachColorTexture(mpDefferedRenderResult);
		mpFboDefferedLighting->setDepthTest(false);
	}
	if (mpSsaoResultMap) {
		mpSsaoResultMap->setParam(GL_NEAREST, GL_NEAREST);
		mpSsaoResultMap->create2DMap(mWidth, mHeight, nullptr, GL_RED, GL_RED,GL_FLOAT);
		mpFboSsao->attachColorTexture(mpSsaoResultMap);
	}
	if (mpSsaoBluredMap) {
		mpSsaoBluredMap->setParam(GL_NEAREST, GL_NEAREST);
		mpSsaoBluredMap->create2DMap(mWidth, mHeight, nullptr, GL_RED, GL_RED, GL_FLOAT);
		mpFboSsaoBlured->attachColorTexture(mpSsaoBluredMap);
	}
	if (mpSsaoNoiseMap) {
		//生成在法线法线方向的半球内呈均匀分布的采样向量，
		//让生成的向量靠近像素中心
		std::uniform_real_distribution<GLfloat> randomFloats(0.0, 1.0); // generates random floats between 0.0 and 1.0
		std::default_random_engine generator;
			
		for (unsigned int i = 0; i < 64; ++i)
		{
			glm::vec4 sample(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, randomFloats(generator),0.0f);
			sample = glm::normalize(sample);
			sample *= randomFloats(generator);
			float scale = float(i) / 64.0f;

			auto lerp=[](float a, float b, float f) ->float{return a + f * (b - a);};
			// scale samples s.t. they're more aligned to center of kernel
			scale = lerp(0.1f, 1.0f, scale * scale);
			sample *= scale;
			mSsaoKernel.push_back(sample);
		}

		//生成一个随机的旋转矩阵，用于旋转采样向量
		std::vector<glm::vec3> ssaoNoise;
		for (unsigned int i = 0; i < 16; i++)
		{
			glm::vec3 noise(randomFloats(generator) * 2.0 - 1.0, randomFloats(generator) * 2.0 - 1.0, 0.0f); // rotate around z-axis (in tangent space)
			ssaoNoise.push_back(noise);
		}
		mpSsaoNoiseMap->setParam(GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT);
		mpSsaoNoiseMap->create2DMap(4, 4, reinterpret_cast<unsigned char*>(ssaoNoise.data()), GL_RGBA32F, GL_RGBA, GL_FLOAT);
	}

	if (mpFboTaa && mpTaaPreColorMap[0] && mpTaaPreColorMap[1]) {
		mpTaaPreColorMap[0]->setParam(GL_NEAREST, GL_NEAREST);
		mpTaaPreColorMap[0]->create2DMap(mWidth, mHeight, nullptr, GL_RGB16F, GL_RGB, GL_FLOAT);
		mpTaaPreColorMap[1]->setParam(GL_NEAREST, GL_NEAREST);
		mpTaaPreColorMap[1]->create2DMap(mWidth, mHeight, nullptr, GL_RGB16F, GL_RGB, GL_FLOAT);
		mpFboTaa->attachColorTexture(mpTaaPreColorMap[1]);
	}
		
	mpMeshQuad = std::make_shared<Mesh>(MeshType::MESH_Quad);
	if (mpMeshQuad) {
		mpMeshQuad->loadMesh();
		Resource& res = Resource::getInstance();
		mpSsaoMaterial = res.getMaterial("ssao");
		if (mpSsaoMaterial) {
			mpSsaoMaterial->setTextureForSampler("posMap", mpPosMap);
			mpSsaoMaterial->setTextureForSampler("normalMap", mpNormal);
			mpSsaoMaterial->setTextureForSampler("noiseMap", mpSsaoNoiseMap);
			Ubo::getInstance().update("SampleArray", mSsaoKernel.data(), sizeof(glm::vec4) * mSsaoKernel.size());
		}
		mpSsaoBlurMaterial = res.getMaterial("ssaoBlur");
		if (mpSsaoBlurMaterial) {
			mpSsaoBlurMaterial->setTextureForSampler("ssaoInput", mpSsaoResultMap);
		}
		mpDefLightPassMaterial = res.getMaterialDefferedLightPass(true);
		if (mpDefLightPassMaterial) {
			mpDefLightPassMaterial->setTextureForSampler("posMap", mpPosMap);
			mpDefLightPassMaterial->setTextureForSampler("albedoMap", mpAlbedoMap);
			mpDefLightPassMaterial->setTextureForSampler("normalMap", mpNormal);
			mpDefLightPassMaterial->setTextureForSampler("ssaoMap", mpSsaoBluredMap);
			if (pScene) {
				auto& skyInfo = pScene->getSkybox();
				mpDefLightPassMaterial->setTextureForSampler("irrMap", skyInfo.mpIrrTex);
				mpDefLightPassMaterial->setTextureForSampler("prefilterMap", skyInfo.mpPrefilterTex);
			}
			mpDefLightPassMaterial->setTextureForSampler("brdfLUT", Resource::getInstance().getTexture("brdfLUT"));
		}

		mpTaaMaterial = res.getMaterial("taa");
		if (mpTaaMaterial) {
			//previousColor map is set dynamicly
			mpTaaMaterial->setTextureForSampler("currentColor", mpDefferedRenderResult);
			mpTaaMaterial->setTextureForSampler("velocityTexture", mpTaaVelocityMap);
			mpTaaMaterial->setTextureForSampler("currentDepth", mpDepthMap);
		}

		mpDrawQuadMaterial = res.getMaterial("drawQuad");
		if (mpDrawQuadMaterial) {
			mpDrawQuadMaterial->setTextureForSampler("albedoMap", mpDefferedRenderResult);
		}
	}
}

void Camera::defferedGeometryPass(const std::shared_ptr<Scene>& pScene) const{
	if (pScene) {
		const auto& rootNode = pScene->getRootDeffered();
		renderNode(rootNode, pScene, nullptr, nullptr);
	}
}

void Camera::updateWidthHeight(int w,int h) {
	mWidth = w;
	mHeight = h;
	perspective(fov, static_cast<float>(w)/static_cast<float>(h), nearp, farp);
}

std::shared_ptr<Scene> Camera::getScene() {
	return mpScene.lock();
}
