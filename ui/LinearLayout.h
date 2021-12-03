#ifndef _LINEAR_LAYOUT_H_
#define _LINEAR_LAYOUT_H_
#include "View.h"
using namespace std;
class UiRender;
class LinearLayout :public View {
public:
	friend UiRender;
	using View::View;//继承基类的构造函数

	bool isVertical() {
		return mOrientation == LayoutParam::Vertical;
	}

	bool isHorizontal() {
		return mOrientation == LayoutParam::Horizontal;
	}

	void draw() override;

	int getChildrenTotalWidthPercent() override;

	int getChildrenTotalHeightPercent() override;

	void getWidthAccordChildren() override;

	void getHeightAccordChildren() override;

	void calcChildPos() override;

	void orientationHandler(const string& content) override;
private:
	int mOrientation{ LayoutParam::Horizontal };
	int mTotalWidthPercent{ 0 };
	int mTotalHeightPercent{ 0 };
};
#endif