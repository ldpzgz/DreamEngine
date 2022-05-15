#pragma once

#include "LinearLayout.h"
/*
* 如果ScrollView有很多个子view，
* 继承MoveListener，让子view监听移动，不必遍历每个子view，一个一个move子view。
*/
class ScrollView :public LinearLayout,public MoveListener {
public:
	using LinearLayout::LinearLayout;//继承基类的构造函数
	ScrollView() = default;
	ScrollView(const ScrollView&) = default;
	void draw() override;

	bool mouseMove(int x, int y,bool notInside) override;

	bool mouseLButtonDown(int x, int y, bool notInside) override;

	bool mouseLButtonUp(int x, int y, bool notInside) override;

	void afterGetWidthHeight() override;

	std::shared_ptr<View> clone() override;
private:
	bool mbLButtonDown{ false };
	glm::ivec2 mPrePos{0,0};
	glm::ivec2 mChildTotalWidthHeight{0,0};
};
