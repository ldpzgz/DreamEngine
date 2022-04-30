#ifndef _TREE_VIEW_H_
#define _TREE_VIEW_H_
#include "View.h"
#include "ViewAdapter.h"
/*
* TreeView 必须要有确定的宽度和高度，不能是wrapContent
*/
class TreeView :public View{
public:
	TreeView();
	explicit TreeView(const std::shared_ptr<View> pParent);
	TreeView(const TreeView&) = default;

	std::shared_ptr<View> clone() override;

	void setAdapter(const std::shared_ptr<ViewAdapter>& pAdapter);

	std::shared_ptr<ViewAdapter>& getAdapter() {
		return mpAdapter;
	}

	void getFirstVisibleItem(int& firstItem,int& hideLength) {
		firstItem = mFirstVisibleItem;
		hideLength = mFirstItemHideLength;
	}

	int getVisibleItemCount();

	void draw() override;
	/*
	* 在这里调用tryToMove，处理treeview的上下拖动
	*/
	bool mouseMove(int x, int y,bool notInside) override;

	bool mouseLButtonDown(int x, int y, bool notInside) override;

	bool mouseLButtonUp(int x, int y, bool notInside) override;

	void orientationHandler(const string& content) override;
private:
	/*
	* 尝试移动moveDistance的距离。
	* 如果移动了，计算出listview里面的第一个和最后一个可见的item
	* 反回值表示是否有移动
	*/
	bool tryToMove(int moveDistance);

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