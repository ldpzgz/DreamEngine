#include "Camera.h"
#include "Scene.h"
#include "Light.h"
#include <glm/gtc/matrix_transform.hpp>
Camera::Camera(const shared_ptr<Scene>& ps):
	Node<glm::mat4>(),
	mpScene(ps)
{
	perspective(fov, aspect, nearp, farp);
}

Camera::Camera(const shared_ptr<Scene>& ps, float asp) :
	Node<glm::mat4>(),
	mpScene(ps),
	aspect(asp)
{
	perspective(fov, aspect,nearp,farp);
}

Camera::~Camera()
{
}

void Camera::perspective(float fovy, float asp, float n, float f) {
	mProjMatrix = glm::perspective(fovy, asp,n,f);
	aspect = asp;
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
		const auto& rootNode = pScene->getRoot();
		renderNode(rootNode, pScene);
	}
}

void Camera::renderNode(const shared_ptr<Node<glm::mat4>>& node, const std::shared_ptr<Scene>& pScene) const
{
	if (node) {
		const auto& pAttaches = node->getAttachments();
		glm::mat4 modelViewMatrix = mMat * node->getWorldMatrix();
		glm::mat4 mvpMatrix = mProjMatrix * modelViewMatrix;
		std::vector<Vec3> lightPos;
		std::vector<Vec3> lightColor;
		if (pScene) {
			auto& lights = pScene->getLights();
			for (auto& pl : lights) {
				if (pl) {
					lightPos.emplace_back(pl->getPosOrDir());
					lightColor.emplace_back(pl->getLightColor());
				}
			}
		}

		for (const auto& pAttach : pAttaches) {
			std::shared_ptr<Mesh> pMesh = std::dynamic_pointer_cast<Mesh>(pAttach.second);
			if (pMesh) {
				pMesh->render(mvpMatrix, modelViewMatrix, lightPos, lightColor, mPosition);
			}
		}
		
		const auto& pChildNodes = node->getChildren();
		for (const auto& pNode : pChildNodes) {
			renderNode(pNode.second,pScene);
		}
	}
}

void Camera::deferredRenderScene() {
	auto scene = mpScene.lock();
	if (scene) {
		const auto& rootNode = scene->getRoot();
		geometryPass(rootNode);
	}
}

void Camera::geometryPass(const shared_ptr<Node<glm::mat4>>& node) const{

}

void Camera::lightingPass(const shared_ptr<Scene>& pScene) {

}

void Camera::setAspect(float asp) {
	aspect = asp;
	perspective(fov, aspect, nearp, farp);
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
