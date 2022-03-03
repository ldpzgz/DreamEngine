#include "Camera.h"
#include "Rect.h"
#include "Scene.h"
#include "Light.h"
#include "Fbo.h"
#include "Log.h"
#include <glm/gtc/matrix_transform.hpp>

Camera::Camera(const shared_ptr<Scene>& ps, int w,int h) :
	mWidth(w),
	mHeight(h),
	Node<glm::mat4>(),
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

void Camera::renderScene() {
	auto pScene = mpScene.lock();
	if (pScene) {
		//获取场景中的灯光
		std::vector<Vec3> lightPos;
		std::vector<Vec3> lightColor;
		const auto& lights = pScene->getLights();
		for (const auto& pl : lights) {
			if (pl) {
				auto& pos = pl->getPosOrDir();
				auto tpos = mMat * glm::vec4(pos.x, pos.y, pos.z,1.0f);
				lightPos.emplace_back(Vec3(tpos.x,tpos.y,tpos.z));
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

void Camera::renderNode(const shared_ptr<Node<glm::mat4>>& node, 
	const std::shared_ptr<Scene>& pScene,
	std::vector<Vec3>* lightPos, 
	std::vector<Vec3>* lightColor) const
{
	if (node) {
		const auto& pAttaches = node->getAttachments();
		glm::mat4 modelViewMatrix = mMat * node->getWorldMatrix();
		glm::mat4 mvpMatrix = mProjMatrix * modelViewMatrix;

		for (const auto& pAttach : pAttaches) {
			std::shared_ptr<Mesh> pMesh = std::dynamic_pointer_cast<Mesh>(pAttach.second);
			if (pMesh) {
				pMesh->render(&mvpMatrix, &modelViewMatrix, nullptr,lightPos, lightColor, &mPosition);
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
			auto& pMat = Material::getMaterial("defferedLighting");
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
				pMat->setTextureForSampler("brdfLUT", Material::getTexture("brdfLUT"));
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

void Camera::defferedLightingPass(std::vector<Vec3>* lightPos,std::vector<Vec3>* lightColor) {
	GLboolean preDepthTest = true;
	glGetBooleanv(GL_DEPTH_TEST, &preDepthTest);
	glDisable(GL_DEPTH_TEST);
	mpMeshQuad->render(nullptr,nullptr,nullptr,lightPos,lightColor,nullptr);
	if (preDepthTest) {
		glEnable(GL_DEPTH_TEST);
	}
}

void Camera::updateWidthHeight(int w,int h) {
	mWidth = w;
	mHeight = h;
	perspective(fov, static_cast<float>(w)/static_cast<float>(h), nearp, farp);
}

void Camera::lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up) {
	Node<glm::mat4>::lookAt(eyepos, center, up);
}
//这里相机的移动，旋转，应该是相反的，因为如果相机往右移动，看到的物体是在往左移动。
void Camera::translate(float x, float y, float z) {
	Node<glm::mat4>::translate(-x, -y, -z);
}

void Camera::rotate(float angle, const glm::vec3& vec) {
	Node<glm::mat4>::rotate(-angle, vec);
}

std::shared_ptr<Scene> Camera::getScene() {
	return mpScene.lock();
}
