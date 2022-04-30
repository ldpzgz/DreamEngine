#ifndef _LIST_ADAPTER_H_
#define _LIST_ADAPTER_H_
#include<string>
#include<vector>
#include "../ViewAdapter.h"
class ListAdapter :public ViewAdapter {
public:
	ListAdapter(std::shared_ptr<View>& pContainerView);

	void setData(std::vector<std::string>&& data);

	std::shared_ptr<View> getView(int position) override;

	Rect<int>* getViewRect(int position) override;

	int getVisibleItemCount() override {
		return mLastVisibleItem - mFirstVisibleItem + 1;
	}

	int getFirstItemHideLength() override {
		return mFirstItemHideLength;
	}

	size_t getItemCounts() override;

	/*
	* 确定第一个和最后一个可见的item，以及他们两个各自的不可见长度
	* 返回值表示是否可以移动
	*/
	bool getVisibleItems(bool fromFirstItem = true) override;
	/*
	* 尝试移动moveDistance的距离。
	* 如果移动了，计算出listview里面的第一个和最后一个可见的item
	* 反回值表示是否有移动
	*/
	bool tryToMove(int moveDistance, bool isVertical) override;

	void forVisibleItems(const std::function<bool(std::shared_ptr<View>&)>&) override;
private:
	void initItemView();
	int mFirstVisibleItem{ 0 };
	int mFirstItemHideLength{ 0 };
	int mLastVisibleItem{ -1 };
	int mLastItemHideLength{ 0 };
	std::vector<std::shared_ptr<View>> mVisibleViews;
	std::shared_ptr<View> mpItemView;
	std::vector<std::string> mDatas;
	//
};
#endif
