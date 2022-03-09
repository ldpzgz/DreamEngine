#ifndef _NODE_ROAMER_H_
#define _NODE_ROAMER_H_
#include "Node.h"
/*
* 操纵节点漫游
*/
class NodeRoamer {
public:
	void setTarget(std::shared_ptr<Node<glm::mat4>>& pNode, std::shared_ptr<Node<glm::mat4>>& pView);
	void startRotate(int x, int y);
	void rotate(int x, int y);
	void endRotate(int x, int y);
	void move(bool front);
protected:
	int mStartRotateX{ 0 };
	int mStartRotateY{ 0 };
	bool mIsStartRotate{ false };
	std::shared_ptr<Node<glm::mat4>> mpNode;
	std::shared_ptr<Node<glm::mat4>> mpView;
};
#endif