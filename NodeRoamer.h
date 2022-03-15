#ifndef _NODE_ROAMER_H_
#define _NODE_ROAMER_H_
#include <memory>
/*
* 操纵节点漫游
*/
class Camera;
class NodeRoamer {
public:
	void setTarget(std::shared_ptr<Node>& pNode, std::shared_ptr<Camera>& pView);
	void startRotate(int x, int y);
	void rotate(int x, int y);
	void endRotate(int x, int y);
	void move(bool front);
protected:
	int mStartRotateX{ 0 };
	int mStartRotateY{ 0 };
	bool mIsStartRotate{ false };
	std::shared_ptr<Node> mpNode;
	std::shared_ptr<Camera> mpView;
};
#endif