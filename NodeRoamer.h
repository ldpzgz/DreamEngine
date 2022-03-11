#ifndef _NODE_ROAMER_H_
#define _NODE_ROAMER_H_
/*
* 操纵节点漫游
*/
class Node;
class NodeRoamer {
public:
	void setTarget(std::shared_ptr<Node>& pNode, std::shared_ptr<Node>& pView);
	void startRotate(int x, int y);
	void rotate(int x, int y);
	void endRotate(int x, int y);
	void move(bool front);
protected:
	int mStartRotateX{ 0 };
	int mStartRotateY{ 0 };
	bool mIsStartRotate{ false };
	std::shared_ptr<Node> mpNode;
	std::shared_ptr<Node> mpView;
};
#endif