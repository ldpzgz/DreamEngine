#ifndef _LIST_VIEW_H_
#define _LIST_VIEW_H_
#include "View.h"
#include "ViewAdapter.h"
/*
* ListView 必须要有确定的宽度和高度，不能是wrapContent
* ListView 里面的textView的尺寸建议是固定尺寸，否则每次text的变动
* 会导致text View的parent View重新计算尺寸calcRect
*/
/*
* todo：对于触摸屏，点击item与滑动listview是有冲突的，要解决这个冲突，
* 需要考虑下面的情况：
* 一 touchDown，touchMove事件的处理逻辑：
* touchDown的时候需要记录按下时间，然后开一个定时器设置detalTime超时，
* 分两种情况：
* 1 在下一个touchMove消息里面，1 判断move距离超过了delta，就是list的滑动事件，取消定时器。
*	没有超过delta，判断距离按下的时间有没有超过deltaTime，有超过就是item Buttondown，
*	没有超过什么都不做
* 2 定时器超时函数被调用，这个时候是item buttonDown事件，更改按钮外观为buttonDown，
*	几下down的xy坐标
*
* 3 在touchMove消息里面，如果某个item已经判定为touchDown了，
	判断与touchdown的pos的距离，如果超过了一个delta值，取消item的touchDown状态，
	改为listView的滑动。
 
  4 touchUp消息处理：
	1 如果某个item是touchDown状态，触发item的onClick
	2 如果是listView的滑动状态，滑动结束
* 
* 
*/
class ListView :public View{
public:
	using View::View;

	ListView() = default;

	ListView(const ListView&) = default;

	std::shared_ptr<View> clone() override;
	
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

	int getVisibleItemCount();

	void draw() override;
	/*
	* 在这里调用tryToMove，处理listview的上下或者左右拖动
	*/
	bool mouseMove(int x, int y,bool notInside) override;

	bool mouseLButtonDown(int x, int y, bool notInside) override;

	bool mouseLButtonUp(int x, int y, bool notInside) override;
private:
	bool mbLButtonDown{ false };
	glm::ivec2 mPrePos{0,0};
	int mFirstVisibleItem{ 0 };
	int mFirstItemHideLength{ 0 };
	int mLastVisibleItem{ 0 };
	int mLastItemHideLength{ 0 };
	bool mbMovable{ false };
	int mVisibleItemTotalLength{ 0 };//所有可见的item加起来的length。
	std::shared_ptr<ViewAdapter> mpAdapter;
};

#endif