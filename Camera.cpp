#include "Rect.h"
#include "Camera.h"
#include "Mesh.h"
#include "Scene.h"
#include "Light.h"
#include "Fbo.h"
#include "Log.h"
#include "Resource.h"
#include <glm/ext/matrix_transform.hpp>

Camera::Camera(const shared_ptr<Scene>& ps, int w,int h) :
	mWidth(w),
	mHeight(h),
	Node(),
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

const glm::mat4& Camera::getViewMatrix() {
	auto worldMatrix = getWorldMatrix();
	mViewMatrix[0][0] = worldMatrix[0][0];
	mViewMatrix[0][1] = worldMatrix[1][0];
	mViewMatrix[0][2] = worldMatrix[2][0];

	mViewMatrix[1][0] = worldMatrix[0][1];
	mViewMatrix[1][1] = worldMatrix[1][1];
	mViewMatrix[1][2] = worldMatrix[2][1];

	mViewMatrix[2][0] = worldMatrix[0][2];
	mViewMatrix[2][1] = worldMatrix[1][2];
	mViewMatrix[2][2] = worldMatrix[2][2];

	mViewMatrix[3][0] = -worldMatrix[3][0] * mViewMatrix[0][0] - worldMatrix[3][1] * mViewMatrix[1][0] - worldMatrix[3][2] * mViewMatrix[2][0];
	mViewMatrix[3][1] = -worldMatrix[3][0] * mViewMatrix[0][1] - worldMatrix[3][1] * mViewMatrix[1][1] - worldMatrix[3][2] * mViewMatrix[2][1];
	mViewMatrix[3][2] = -worldMatrix[3][0] * mViewMatrix[0][2] - worldMatrix[3][1] * mViewMatrix[1][2] - worldMatrix[3][2] * mViewMatrix[2][2];
	
	return mViewMatrix;
}

void Camera::lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up) noexcept {
	mViewMatrix = glm::lookAt(eyepos, center, up);
	mMat[0][0] = mViewMatrix[0][0];
	mMat[0][1] = mViewMatrix[1][0];
	mMat[0][2] = mViewMatrix[2][0];

	mMat[1][0] = mViewMatrix[0][1];
	mMat[1][1] = mViewMatrix[1][1];
	mMat[1][2] = mViewMatrix[2][1];

	mMat[2][0] = mViewMatrix[0][2];
	mMat[2][1] = mViewMatrix[1][2];
	mMat[2][2] = mViewMatrix[2][2];

	mMat[3][0] = -mViewMatrix[3][0] * mMat[0][0] - mViewMatrix[3][1] * mMat[1][0] - mViewMatrix[3][2] * mMat[2][0];
	mMat[3][1] = -mViewMatrix[3][0] * mMat[0][1] - mViewMatrix[3][1] * mMat[1][1] - mViewMatrix[3][2] * mMat[2][1];
	mMat[3][2] = -mViewMatrix[3][0] * mMat[0][2] - mViewMatrix[3][1] * mMat[1][2] - mViewMatrix[3][2] * mMat[2][2];
}

void Camera::renderScene() {
	auto pScene = mpScene.lock();
	if (pScene) {
		getViewMatrix();
		//获取场景中的灯光
		std::vector<glm::vec3> lightPos;
		std::vector<glm::vec3> lightColor;
		const auto& lights = pScene->getLights();
		for (const auto& pl : lights) {
			if (pl) {
				auto& pos = pl->getPosOrDir();
				auto tpos = mViewMatrix * glm::vec4(pos.x, pos.y, pos.z,1.0f);
				lightPos.emplace_back(glm::vec3(tpos.x,tpos.y,tpos.z));
				lightColor.emplace_back(pl->getLightColor());
			}
		}
		
		//deffered rendering
		if (!mpFboDefferedGeo) {
			initDefferedRendering(pScene);
		}
		if (mpFboDefferedGeo) {
			mpFboDefferedGeo->render([this, &pScene] {
				defferedGeometryPass(pScene);
				});
			mpFboDefferedGeo->blitDepthBufToWin();

			//deffered render lighting pass
			defferedLightingPass(&lightPos, &lightColor);
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
		glm::mat4 modelViewMatrix = mViewMatrix * node->getWorldMatrix();

		for (const auto& pRen : pRenderables) {
			//std::shared_ptr<R> pMesh = std::dynamic_pointer_cast<Mesh>(pRen.second);
			if (pRen.second) {
				pRen.second->draw(&mProjMatrix, &modelViewMatrix, nullptr,lightPos, lightColor, &mPosition);
			}
		}
		
		const auto& pChildNodes = node->getChildren();
		for (const auto& pNode : pChildNodes) {
			renderNode(pNode.second,pScene, lightPos, lightColor);
		}
	}
}

void Camera::initDefferedRendering(const std::shared_ptr<Scene>& pScene) {
	try {
		mpFboDefferedGeo = make_shared<Fbo>();
		mpPosMap = std::make_shared<Texture>();//0
		mpNormal = std::make_shared<Texture>();//1
		mpAlbedoMap = std::make_shared<Texture>();//2
		if (mpPosMap && mpNormal && mpAlbedoMap) {
			mpPosMap->create2DMap(mWidth, mHeight, nullptr, GL_RGBA16F, GL_RGBA, GL_FLOAT);
			mpNormal->create2DMap(mWidth, mHeight, nullptr, GL_RGBA16F, GL_RGBA, GL_FLOAT);
			mpAlbedoMap->create2DMap(mWidth, mHeight, nullptr, GL_RGBA16F, GL_RGBA, GL_FLOAT);
		}

		mpMeshQuad = std::make_shared<Mesh>(MeshType::MESH_Quad);
		if (mpMeshQuad) {
			mpMeshQuad->loadMesh();
			/*auto& pMat1 = Material::getMaterial("defferedTest");
			if (pMat1) {
				pMat1->setTextureForSampler("posMap", mpNormal);
				mpMeshQuad->setMaterial(pMat1);
			}*/
			auto& pMat = Resource::getInstance().getMaterial("defferedLighting");
			if (pMat) {
				mpMeshQuad->setMaterial(pMat);
				pMat->setTextureForSampler("posMap", mpPosMap);
				pMat->setTextureForSampler("albedoMap", mpAlbedoMap);
				pMat->setTextureForSampler("normalMap", mpNormal);
				if (pScene) {
					auto& skyInfo = pScene->getSkybox();
					pMat->setTextureForSampler("irrMap", skyInfo.mpIrrTex);
					pMat->setTextureForSampler("prefilterMap", skyInfo.mpPrefilterTex);
				}
				pMat->setTextureForSampler("brdfLUT", Resource::getInstance().getTexture("brdfLUT"));
			}
		}
		
	}
	catch (...) {
		LOGE(" %s create mpFboDeffered and maps failed",__func__);
		return;
	}
	mpFboDefferedGeo->attachColorTexture(mpPosMap, 0);
	mpFboDefferedGeo->attachColorTexture(mpNormal, 1);
	mpFboDefferedGeo->attachColorTexture(mpAlbedoMap, 2);
	mpFboDefferedGeo->attachDepthRbo(mWidth, mHeight);//深度缓存
}

void Camera::defferedGeometryPass(const std::shared_ptr<Scene>& pScene) const{
	if (pScene) {
		const auto& rootNode = pScene->getRootDeffered();
		renderNode(rootNode, pScene, nullptr, nullptr);
	}
}

void Camera::defferedLightingPass(std::vector<glm::vec3>* lightPos,std::vector<glm::vec3>* lightColor) {
	GLboolean preDepthTest = true;
	glGetBooleanv(GL_DEPTH_TEST, &preDepthTest);
	glDisable(GL_DEPTH_TEST);
	mpMeshQuad->draw(nullptr,nullptr,nullptr,lightPos,lightColor);
	if (preDepthTest) {
		glEnable(GL_DEPTH_TEST);
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
