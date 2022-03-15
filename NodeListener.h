#pragma once
#include <glm/mat4x4.hpp>
/*
* 节点方位变化监听
*/
class NodeListener {
public:
	virtual void update(const glm::mat4& mat) = 0;
};