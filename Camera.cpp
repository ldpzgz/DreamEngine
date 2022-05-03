#include "Node.h"
#include "Rect.h"
#include "Camera.h"
#include "Material.h"
#include "Mesh.h"
#include "Light.h"
#include "Scene.h"
#include "Shader.h"
#include "Fbo.h"
#include "Log.h"
#include "Resource.h"
#include "Config.h"
#include "PostSmaa.h"
#include "PostToneMap.h"
#include "PostGsBlur.h"
#include "Ubo.h"
#include "Sampler.h"
//#include <glm/trigonometric.hpp>  //sin cos,tan,radians,degree
#include <glm/ext/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp> // value_ptr,make_vec,make_mat
#include <random>
#include "AnimationManager.h"

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

void Camera::updateUboForShader() {
	Ubo& ubo = Ubo::getInstance();
	//更新proj，view矩阵
	glm::mat4 projView[2]{ mProjMatrix,mViewMat };
	ubo.update("Matrixes", projView,2*sizeof(glm::mat4),0);
	
	//更新屏幕宽高
	float temp[2]{ mWidth ,mHeight };
	ubo.update("ScreenWH", temp, sizeof(temp));

	if (Config::openTaa) {
		int taaUniform[2];
		taaUniform[0] = mTaaFrameCount++;
		taaUniform[1] = mTaaOffsetIndex++;
		mTaaOffsetIndex %= 8;
		if (mTaaFrameCount > 100000000) {
			//避免长时间运行溢出
			mTaaFrameCount = 1;
		}
		ubo.update("Taa", taaUniform, sizeof(taaUniform));
	}
}

void Camera::renderScene() {
	auto pScene = mpScene.lock();
	if (pScene) {
		
		//deffered rendering
		if (!mpFboDefferedGeo) {
			initDefferedRendering(pScene);
		}
		if (!mpFboForward) {
			mpFboForward = std::make_shared<Fbo>();
			mpFboForward->attachColorTexture(mpRenderResult);
			mpFboForward->attachDepthTexture(mpDepthMap);//深度缓存
			mpFboForward->setClearColor(false);
			mpFboForward->setClearDepth(false);//depth is enable,but do not clear depth at first
		}

		//render ShadowMap
		genShadowMap(pScene);

		updateUboForShader();
		pScene->updateLightsForShader(mViewMat);
		if (mpFboDefferedGeo) {
			mpFboDefferedGeo->render([this, &pScene] {
				//deffered rendering geometry pass
				defferedGeometryPass(pScene);
				});
			//ssao
			if (mpFboSsao) {
				mpFboSsao->render([this] {
					mpPostMesh->setMaterial(mpSsaoMaterial);
					mpPostMesh->draw(nullptr);
					});
			}
			//ssao blur
			if (mpFboSsaoBlured) {
				mpFboSsaoBlured->render([this] {
					mpPostMesh->setMaterial(mpSsaoBlurMaterial);
					mpPostMesh->draw(nullptr, nullptr);
					});
			}

			//blur the shadowResult
			mpPostGsBlur->process(mpShadowResult, mpShadowResult1);
			mpPostGsBlur->process(mpShadowResult1, mpShadowResult);

			//deffered render lighting pass
			if (mpFboDefferedLighting) {
				mpFboDefferedLighting->render([this]() {
					mpPostMesh->setMaterial(mpDefLightPassMaterial);
					mpPostMesh->draw(nullptr, nullptr, nullptr);
					});
			}
		}
		//forward rendering
		mpFboForward->render([this,&pScene]() {
			const auto& rootNode = pScene->getRoot();
			renderNode(rootNode, pScene, nullptr, nullptr);
			});


		//post-process
			//taa
		if (Config::openTaa && mpTaaMaterial) {
			//previousColor map is set dynamicly
			mpTaaMaterial->setTextureForSampler("previousColor", mpTaaPreColorMap[mTaaPreColorMapIndex]);
			mTaaPreColorMapIndex = (mTaaPreColorMapIndex + 1) % 2;
			mpFboTaa->replaceColorTexture(mpTaaPreColorMap[mTaaPreColorMapIndex], 0);
			mpPostMesh->setMaterial(mpTaaMaterial);
			mpFboTaa->render([this]() {
				mpPostMesh->draw(nullptr, nullptr);
				});

			if (mpDrawQuadMaterial) {
				mpDrawQuadMaterial->setTextureForSampler("albedoMap", mpTaaPreColorMap[mTaaPreColorMapIndex]);
			}
		}

		if (mpPostTonemap) {
			mpPostTonemap->process(mpRenderResult, mpPostTex[0]);
			if (mpDrawQuadMaterial) {
				mpDrawQuadMaterial->setTextureForSampler("albedoMap", mpPostTex[0]);
			}
		}

		if (Config::openSmaa) {
			mpPostSmaa->process(mpPostTex[0], mpPostTex[1]);
			if (mpDrawQuadMaterial) {
				mpDrawQuadMaterial->setTextureForSampler("albedoMap", mpPostTex[1]);
			}
		}
		
		
		if (mpDrawQuadMaterial) {
			mpDrawQuadMaterial->setTextureForSampler("albedoMap", mpPostTex[1]);
		}
		//在窗口系统将最终渲染结果绘制出来
		mpPostMesh->setMaterial(mpDrawQuadMaterial);
		mpPostMesh->draw(nullptr, nullptr);
		
		//把深度缓冲区copy到窗口系统
		mpFboForward->blitDepthBufToWin();
	}
}

void Camera::renderNode(const shared_ptr<Node>& pRootNode, 
	const std::shared_ptr<Scene>& pScene,
	std::vector<glm::vec3>* lightPos, 
	std::vector<glm::vec3>* lightColor) const noexcept
{
	if (pRootNode) {
		pRootNode->visitNode([this, lightPos, lightColor, &pScene](Node* node) {
			const auto& pRenderables = node->getRenderables();
			if (pRenderables.empty()) {
				return;
			}
			glm::mat4 modelMat = node->getWorldMatrix();
			for (const auto& pRen : pRenderables) {
				if (pRen.second) {
					if (Config::openShadowMap) {
						std::shared_ptr<Mesh> pMesh = std::dynamic_pointer_cast<Mesh>(pRen.second);
						if (pMesh && pMesh->getReceiveShadow()) {
							auto& pMaterial = pMesh->getMaterial();
							if (pMaterial) {
								pMaterial->setTextureForSampler("shadowMap", mpShadowMap);
							}
						}
					}
					if (Config::openTaa) {
						glm::mat4 projView = mProjMatrix * mViewMat;
						pRen.second->draw(&modelMat, nullptr, &projView);
					}
					else {
						pRen.second->draw(&modelMat, nullptr, nullptr);
					}
				}
			}
		});
	}
}

void Camera::initDefferedRendering(const std::shared_ptr<Scene>& pScene) noexcept {
	mpFboDefferedGeo = make_shared<Fbo>();
	mpFboDefferedLighting = make_shared<Fbo>();
	mpFboSsao = make_shared<Fbo>();
	mpFboSsaoBlured = make_shared<Fbo>();
	mpFboTaa = make_shared<Fbo>();
	mpPosMap = std::make_shared<Texture>();//0
	mpNormal = std::make_shared<Texture>();//1
	mpAlbedoMap = std::make_shared<Texture>();//2
	mpDepthMap = std::make_shared<Texture>();
	mpRenderResult = std::make_shared <Texture>();
	mpSsaoResultMap = std::make_shared<Texture>();
	mpSsaoNoiseMap = std::make_shared<Texture>();
	mpSsaoBluredMap = std::make_shared<Texture>();
	mpTaaVelocityMap = std::make_shared<Texture>();
	mpTaaPreColorMap[0] = std::make_shared<Texture>();
	mpTaaPreColorMap[1] = std::make_shared<Texture>();
	mpPostTex[0] = std::make_shared<Texture>();
	mpPostTex[1] = std::make_shared<Texture>();
	auto pNearestSampler = Sampler::getSampler(SamplerType::NearNearEdgeEdge);
	auto pLinearSampler = Sampler::getSampler(SamplerType::LinearLinearEdgeEdge);
	auto pLBSampler = Sampler::getSampler(SamplerType::LinearLinearBorderBorder);
	auto pNearestRepeatSampler = Sampler::getSampler(SamplerType::NearNearRepeatRepeat);
	if (mpPosMap && mpNormal && mpAlbedoMap && mpDepthMap && mpTaaVelocityMap) {
		mpPosMap->setSampler(pNearestSampler);
		mpPosMap->create2DMap(mWidth, mHeight, nullptr, GL_RGBA16F, GL_RGBA, GL_FLOAT);
		
		mpNormal->setSampler(pNearestSampler);
		mpNormal->create2DMap(mWidth, mHeight, nullptr, GL_RGBA16F, GL_RGBA, GL_FLOAT);
		
		mpAlbedoMap->setSampler(pLinearSampler);
		mpAlbedoMap->create2DMap(mWidth, mHeight, nullptr, GL_RGBA16F, GL_RGBA, GL_FLOAT);
		
		mpDepthMap->setSampler(pNearestSampler);
		mpDepthMap->create2DMap(mWidth, mHeight, nullptr, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
		
		mpTaaVelocityMap->setSampler(pNearestSampler);
		mpTaaVelocityMap->create2DMap(mWidth, mHeight, nullptr, GL_RG16F, GL_RG, GL_FLOAT);
		
		mpFboDefferedGeo->attachColorTexture(mpPosMap, 0);
		mpFboDefferedGeo->attachColorTexture(mpNormal, 1);
		mpFboDefferedGeo->attachColorTexture(mpAlbedoMap, 2);
		if (Config::openShadowMap) {
			mpShadowResult = std::make_shared<Texture>();
			mpShadowResult->setSampler(pLBSampler);
			mpShadowResult->create2DMap(mWidth, mHeight, nullptr, GL_R8, GL_RED, GL_UNSIGNED_BYTE);
			
			mpShadowResult1 = std::make_shared<Texture>();
			mpShadowResult1->setSampler(pLBSampler);
			mpShadowResult1->create2DMap(mWidth, mHeight, nullptr, GL_R8, GL_RED, GL_UNSIGNED_BYTE);
		}
		if (Config::openTaa) {
			mpFboDefferedGeo->attachColorTexture(mpTaaVelocityMap, 3);
			if (Config::openShadowMap) {
				mpFboDefferedGeo->attachColorTexture(mpShadowResult, 4);
			}
		}
		else {
			if (Config::openShadowMap) {
				mpFboDefferedGeo->attachColorTexture(mpShadowResult, 3);
			}
		}
		
		mpFboDefferedGeo->attachDepthTexture(mpDepthMap);//深度缓存
	}
	if (mpRenderResult) {
		mpRenderResult->create2DMap(mWidth,mHeight,nullptr, GL_RGBA16F, GL_RGBA,GL_FLOAT);
		mpFboDefferedLighting->attachColorTexture(mpRenderResult);
		mpFboDefferedLighting->setDepthTest(false);
	}
	if (mpSsaoResultMap) {
		mpSsaoResultMap->setSampler(pNearestSampler);
		mpSsaoResultMap->create2DMap(mWidth, mHeight, nullptr, GL_RED, GL_RED,GL_FLOAT);
		mpFboSsao->attachColorTexture(mpSsaoResultMap);
	}
	if (mpSsaoBluredMap) {
		mpSsaoBluredMap->setSampler(pNearestSampler);
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
		mpSsaoNoiseMap->setSampler(pNearestRepeatSampler);
		mpSsaoNoiseMap->create2DMap(4, 4, reinterpret_cast<unsigned char*>(ssaoNoise.data()), GL_RGBA32F, GL_RGBA, GL_FLOAT);
	}

	if (mpFboTaa && mpTaaPreColorMap[0] && mpTaaPreColorMap[1]) {
		mpTaaPreColorMap[0]->setSampler(pNearestSampler);
		mpTaaPreColorMap[0]->create2DMap(mWidth, mHeight, nullptr, GL_RGB16F, GL_RGB, GL_FLOAT);
		
		mpTaaPreColorMap[1]->setSampler(pNearestSampler);
		mpTaaPreColorMap[1]->create2DMap(mWidth, mHeight, nullptr, GL_RGB16F, GL_RGB, GL_FLOAT);
		mpFboTaa->attachColorTexture(mpTaaPreColorMap[1]);
	}
		
	mpPostMesh = std::make_shared<Mesh>(MeshType::TrianglePost);
	if (mpPostMesh) {
		mpPostMesh->loadMesh();
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
			if (Config::openShadowMap) {
				mpDefLightPassMaterial->setTextureForSampler("shadowResult", mpShadowResult);
			}
		}

		if (Config::openTaa) {
			mpTaaMaterial = res.getMaterial("taa");
			if (mpTaaMaterial) {
				//previousColor map is set dynamicly
				mpTaaMaterial->setTextureForSampler("currentColor", mpRenderResult);
				mpTaaMaterial->setTextureForSampler("velocityTexture", mpTaaVelocityMap);
				mpTaaMaterial->setTextureForSampler("currentDepth", mpDepthMap);
			}
		}

		if (mpPostTex[0] && mpPostTex[1]) {
			mpPostTex[0]->setSampler(pLinearSampler);
			mpPostTex[0]->create2DMap(mWidth, mHeight, nullptr, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
			mpPostTex[1]->setSampler(pLinearSampler);
			mpPostTex[1]->create2DMap(mWidth, mHeight, nullptr, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE);
		}
		
		mpPostTonemap = std::make_unique<PostToneMap>();
		mpPostTonemap->initPost(mWidth, mHeight, mpPostMesh);
		mpPostSmaa = std::make_unique<PostSmaa>();
		mpPostSmaa->initPost(mWidth, mHeight, mpPostMesh);

		mpPostGsBlur = std::make_unique<PostGsBlur>();
		mpPostGsBlur->initPost(mWidth, mHeight, mpPostMesh);

		mpDrawQuadMaterial = res.getMaterial("drawQuad");
		if (mpDrawQuadMaterial) {
			mpDrawQuadMaterial->setTextureForSampler("albedoMap", mpRenderResult);
		}
	}
}

void Camera::genShadowMap(std::shared_ptr<Scene>& pScene) {
	if (Config::openShadowMap == 0) {
		return;
	}
	if (!mpShadowMap) {
		constexpr int SM_WIDTH = 1024;
		constexpr int SM_HEIGHT = 1024;
		mpShadowMap = std::make_shared<Texture>();
		float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
		auto pSampler = Sampler::getSampler(SamplerType::LinearLinearBorderBorderEdgeLe);
		mpShadowMap->setSampler(pSampler);
		mpShadowMap->setBorderColor(borderColor);
		mpShadowMap->create2DMap(SM_WIDTH, SM_HEIGHT, nullptr, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT, GL_FLOAT);
		mpGenShadowMaterial = Resource::getInstance().cloneMaterial("genShadowMap");
		mpGenShadowAnimMaterial = Resource::getInstance().cloneMaterial("genShadowMapAnim");
		assert(mpGenShadowMaterial);
		assert(mpGenShadowAnimMaterial);
		mpFboShadowMap = std::make_unique<Fbo>();
		mpFboShadowMap->attachDepthTexture(mpShadowMap,0,true); 
	}
	if (pScene) {
		//找到投射阴影的灯光，建立projection Matrix，viewMat
		auto& lights = pScene->getLights();
		std::shared_ptr<Light> pLight;
		for (auto& pl : lights) {
			if (pl && pl->isDirectionalLight() && pl->getCastShadow())
				pLight = pl;
		}
		if (pLight) {
			auto& direction = glm::normalize(pLight->getPosOrDir());
			glm::mat4 projMat{ 1.0f };
			glm::vec3 lookat(0.0f, 0.0f, -3.0f);
			glm::vec3 lightpos = lookat - direction * 5.0f;
			
			glm::vec3 up = glm::cross(direction, glm::vec3(0.0f, -1.0f, 0.0f));
			up = glm::cross(direction, up);
			glm::mat4 lightViewMat{1.0f};
			lightViewMat = glm::lookAt(lightpos, lookat, up);
			//lightViewMat = lightViewMat*mViewMat;
			projMat = glm::ortho(-3.0f,3.0f,-3.0f,3.0f,2.0f,7.0f);
			mProjViewMatShadow = projMat * lightViewMat;
			glm::mat4 tempMat[2]{ mViewMat, mProjViewMatShadow };
			Ubo::getInstance().update("Matrixes", tempMat, 2 *sizeof(glm::mat4), sizeof(glm::mat4));
			const auto& rootNode = pScene->getRootDeffered();
			mpFboShadowMap->render([this, &rootNode](){
				if (mpGenShadowMaterial) {
					auto& pMat = mpGenShadowMaterial;
					auto& pMatA = mpGenShadowAnimMaterial;
					AnimationManager::getInstance().updateActiveAnimation();
					rootNode->visitNode([&pMat,&pMatA](Node* pNode) {
						const auto& pRenderables = pNode->getRenderables();
						if (pRenderables.empty()) {
							return;
						}
						glm::mat4 modelMat = pNode->getWorldMatrix();
						for (const auto& pRen : pRenderables) {
							auto& renderable = pRen.second;
							if (renderable) {
								if (renderable->hasAnimation()) {
									pMatA->enable();
									pMatA->getShader()->setModelMatrix(modelMat);
									renderable->draw(0, -1, -1, -1, 1, 2);
								}
								else {
									pMat->enable();
									pMat->getShader()->setModelMatrix(modelMat);
									renderable->draw(0, -1, -1, -1, -1, -1);
								}
								
							}
						}
					});
				}
			});
		}
		else {
			return;
		}
	}
}

void Camera::defferedGeometryPass(const std::shared_ptr<Scene>& pScene) const noexcept{
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
