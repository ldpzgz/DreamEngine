#ifndef _NODE_ROAMER_H_
#define _NODE_ROAMER_H_
#include "Node.h"
/*
* ²Ù×Ý½ÚµãÂþÓÎ
*/
class NodeRoamer {
public:
	void setTarget(std::shared_ptr<Node<glm::mat4>>& pNode);
	void startRotate(int x, int y);
	void rotate(int x, int y);
	void endRotate(int x, int y);
	void move(bool front);
protected:
	int mStartRotateX{ 0 };
	int mStartRotateY{ 0 };
	bool mIsStartRotate{ false };
	glm::mat4 mOriginMat{ 1.0f };
	std::shared_ptr<Node<glm::mat4>> mpNode;
};
#endif