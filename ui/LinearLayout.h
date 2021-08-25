#ifndef _LINEAR_LAYOUT_H_
#define _LINEAR_LAYOUT_H_
#include "View.h"
using namespace std;
class UiRender;
class LinearLayout :public View {
public:
	friend UiRender;
	using View::View;//继承基类的构造函数
	LinearLayout(shared_ptr<View> parent, unsigned int layoutParam):
		View(parent),
		mLayoutParam(layoutParam)
	{

	}

	void setLayoutParam(unsigned int layoutParam) {
		mLayoutParam = layoutParam;
	}

	void draw() override;

	int getTotalWidthPercent() override;

	int getTotalHeightPercent() override;

	void calcChildPos() override;

	static void orientationHandler(const shared_ptr<View>&, const string&);
private:
	unsigned int mLayoutParam{ LayoutParam::Horizontal };
	int mOrientation{ 0 };
	int mTotalWidthPercent{ 0 };
	int mTotalHeightPercent{ 0 };
};
#endif