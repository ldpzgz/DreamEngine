#ifndef _LIST_ADAPTER_H_
#define _LIST_ADAPTER_H_
#include<string>
#include<vector>
#include "../ViewAdapter.h"
#include "ListData.h"
class ListAdapter :public ViewAdapter {
public:
	ListAdapter(std::shared_ptr<View>& pContainerView);

	void setData(const std::shared_ptr<ListData>& pData);
	/*
	* override the three func below
	*/
	/*
	* load itemview in this function,
	* set onclick listener for some chid view etc;
	*/
	virtual void initItemView() {
	}
	virtual int getViewTypeCount() {
		return 1;
	}
	/*
	* this func will be called when showing listview
	* you should set the apperance of pView and 
	* attach anydata on pView if you like,in this func;
	*/
	virtual void setViewData(const std::shared_ptr<View>& pView, int type,int dataPos) {

	}
	/*
	* override the three func above
	*/

	
	std::shared_ptr<View> getView(int position) override;
	Rect<int>* getViewRect(int position) override;

	size_t getDataSize() override;

	int getVisibleItemCount() override {
		return mLastVisibleItem - mFirstVisibleItem + 1;
	}
	/*
	* 显示listView的时候会被调用
	*/
	int getFirstItemHideLength() override {
		return mFirstItemHideLength;
	}
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
protected:
	int mFirstVisibleItem{ 0 };
	int mFirstItemHideLength{ 0 };
	int mLastVisibleItem{ -1 };
	int mLastItemHideLength{ 0 };
	std::vector<std::shared_ptr<View>> mVisibleViews;
	std::shared_ptr<ListData> mpDatas;
	std::vector< std::vector<std::shared_ptr<View>> > mItemViews;
	std::vector <int> mItemViewRefCount;
	//
};
#endif
