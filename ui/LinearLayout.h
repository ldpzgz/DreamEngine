#ifndef _LINEAR_LAYOUT_H_
#define _LINEAR_LAYOUT_H_
#include "View.h"
using namespace std;
class UiRender;
class LinearLayout :public View {
public:
	friend UiRender;
	using View::View;

	LinearLayout() = default;

	LinearLayout(const LinearLayout&) = default;

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

	std::shared_ptr<View> clone() override;
private:
	int mTotalWidthPercent{ 0 };
	int mTotalHeightPercent{ 0 };
};
#endif