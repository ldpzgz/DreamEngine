#ifndef _LIST_VIEW_H_
#define _LIST_VIEW_H_
#include "View.h"
#include "ViewAdapter.h"
/*
* ListView 必须要有确定的宽度和高度，不能是wrapContent
*/
class ListView :public View{
public:
	using View::View;//继承基类的构造函数
	
	void setAdapter(const std::shared_ptr<ViewAdapter>& pAdapter);

	std::shared_ptr<ViewAdapter>& getAdapter() {
		return mpAdapter;
	}

	void getFirstVisibleItem(int& firstItem,int& hideLength) {
		firstItem = mFirstVisibleItem;
		hideLength = mFirstItemHideLength;
	}

	void getLastVisibleItem(int& lastItem, int& hideLength) {
		lastItem = mLastVisibleItem;
		hideLength = mLastItemHideLength;
	}

	bool isHorizontal() {
		return mOrientation == LayoutParam::Horizontal;
	}

	void draw() override;

	bool mouseMove(int x, int y) override;

	bool mouseLButtonDown(int x, int y) override;

	bool mouseLButtonUp(int x, int y) override;

	void orientationHandler(const string& content) override;
private:
	/*
	* 算出第一个和最后一个可见的item
	*/
	void getVisibleItems();
	/*
	* 尝试移动moveDistance的距离。
	* 如果移动了，计算出listview里面的第一个和最后一个可见的item
	* 反回值表示是否有移动
	*/
	bool tryToMove(int moveDistance);

	unsigned int mMouseState{ MouseState::MouseNone };
	bool mbLButtonDown{ false };
	Vec2i mPrePos;
	int mFirstVisibleItem{ 0 };
	int mFirstItemHideLength{ 0 };
	int mLastVisibleItem{ 0 };
	int mLastItemHideLength{ 0 };
	bool mbMovable{ false };
	int mVisibleItemTotalLength{ 0 };//所有可见的item加起来的length。
	int mOrientation{ LayoutParam::Horizontal };
	std::shared_ptr<ViewAdapter> mpAdapter;
};

#endif