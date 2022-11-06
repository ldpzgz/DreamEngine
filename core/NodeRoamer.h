#pragma once

#include <memory>
#include <glm/vec3.hpp>
#include <glm/mat4x4.hpp>
/*
* 操纵节点漫游
*/
class Camera;
class NodeRoamer {
public:
	void setTarget(std::shared_ptr<Node>& pNode, std::shared_ptr<Camera>& pView);//const glm::vec3& translateForScale
	void startRotate(int x, int y);
	void rotate(int x, int y);
	void endRotate(int x, int y);
	void move(bool front);
protected:
	int mStartRotateX{ 0 };
	int mStartRotateY{ 0 };
	float mLengthBase{ 1.0 };
	bool mIsStartRotate{ false };
	std::shared_ptr<Node> mpNode;
	std::shared_ptr<Camera> mpView;
	//glm::mat4 mTranslateForScale{ 1.0f };
	//glm::mat4 mInverseTranslate{ 1.0f };
};
