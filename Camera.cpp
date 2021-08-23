#include "Camera.h"
#include "Scene.h"
#include <glm/gtc/matrix_transform.hpp>
Camera::Camera(const shared_ptr<Scene>& ps):
	Node<glm::mat4>(),
	mpScene(ps),
	mProjMatrix(1.0f),
	mProjViewMatrix(1.0f),
	aspect(4.0f / 3.0f),
	fov(45.0f),
	nearp(0.1f),
	farp(100.0f)
{
	perspective(45.0f, 4.0f / 3.0f, 0.1f, 100.0f);
}

Camera::Camera(const shared_ptr<Scene>& ps, float asp) :
	Node<glm::mat4>(),
	mpScene(ps),
	mProjMatrix(1.0f),
	mProjViewMatrix(1.0f),
	aspect(asp),
	fov(45.0f),
	nearp(0.1f),
	farp(100.0f)
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
	mProjViewMatrix = mProjMatrix*mMat;
}

void Camera::ortho(float left, float right, float bottom, float top, float zNear, float zFar) {
	mProjMatrix = glm::ortho(left,right,bottom,top,zNear,zFar);
	mProjViewMatrix = mProjMatrix*mMat;
}

void Camera::renderScene() {
	auto scene = mpScene.lock();
	if (scene) {
		const auto& rootNode = scene->getRoot();
		renderNode(rootNode);
	}
}

void Camera::renderNode(const shared_ptr<Node<glm::mat4>>& node) const
{
	if (node) {
		const auto& pMeshes = node->getAttachments();
		glm::mat4 worldMatrix = node->getWorldMatrix();
		
		for (const auto& pMesh : pMeshes) {
			std::dynamic_pointer_cast<Mesh>(pMesh.second)->render(mProjViewMatrix * worldMatrix);
		}
		//for_each(pMeshes.cbegin(), pMeshes.cend(), [this,&worldMatrix](const MapIAttachable::value_type& pMesh) {
		//	if (pMesh.second) {
		//		std::dynamic_pointer_cast<Mesh>(pMesh.second)->render(mProjViewMatrix * worldMatrix);//
		//	}
		//});

		const auto& pChildNodes = node->getChildren();
		for (const auto& pNode : pChildNodes) {
			renderNode(pNode.second);
		}
		/*for_each(pChildNodes.cbegin(), pChildNodes.cend(), [this](const MapINode::value_type& pNode) {
			renderNode(pNode.second);
		});*/
	}
}

void Camera::setAspect(float asp) {
	aspect = asp;
	perspective(fov, aspect, nearp, farp);
}

void Camera::lookAt(const glm::vec3& eyepos, const glm::vec3& center, const glm::vec3& up) {
	Node<glm::mat4>::lookAt(eyepos, center, up);
	mProjViewMatrix = mProjMatrix * mMat;
}

void Camera::translate(float x, float y, float z) {
	Node<glm::mat4>::translate(x, y, z);
	mProjViewMatrix = mProjMatrix * mMat;
}

void Camera::rotate(float angle, const glm::vec3& vec) {
	Node<glm::mat4>::rotate(angle, vec);
	mProjViewMatrix = mProjMatrix * mMat;
}
