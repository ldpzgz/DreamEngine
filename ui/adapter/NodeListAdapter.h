#ifndef _NODE_LIST_ADAPTER_H_
#define _NODE_LIST_ADAPTER_H_
#include<string>
#include<vector>
#include "../ViewAdapter.h"
class Node;
class TextView;
class Texture;
class NodeListAdapter :public ViewAdapter {
public:
	using ViewAdapter::ViewAdapter;

	void setNode(std::shared_ptr<Node>& pNode);

	std::shared_ptr<View> getView(int position) override;

	Rect<int>* getViewRect(int position) override;

	size_t getItemCounts() override;

	int getFirstItemHideLength() override {
		return mFirstItemHideLength;
	}

	int getVisibleItemCount() override {
		return mVisibleItemCount;
	}
	
	/*
	* 尝试移动moveDistance的距离。
	* 如果移动了，计算出listview里面的第一个和最后一个可见的item
	* 反回值表示是否有移动
	*/
	bool tryToMove(int moveDistance, bool isVertical) override;

	void forVisibleItems(const std::function<bool(std::shared_ptr<View>&)>&) override;
private:
	void initItemView();
	Node* getViewInternal(int pos,bool toFirst=true);
	/*
	* 统计出可显示的item，返回值表示是否可以移动
	* 移动，展开或者折叠某个item后调用
	*/
	bool getVisibleItems(bool fromFirstItem = true) override;
	std::shared_ptr<View> mpItemView;
	std::shared_ptr<Texture> mpWrapTex;
	std::shared_ptr<Texture> mpUnWrapTex;
	std::shared_ptr<Texture> mpNullTex;
	std::shared_ptr<Texture> mpNodeTex;

	std::shared_ptr<Node> mpRootNode;
	std::shared_ptr<Node> mpFirstVisibleNode;
	std::shared_ptr<Node> mpLastVisibleNode;
	int mItemHeight{0};
	int mFirstItemHideLength{ 0 };
	int mLastItemHideLength{ 0 };
	int mVisibleItemTotalHeight{ 0 };
	int mItemCount{ 0 };
	int mVisibleItemCount{ 0 };
	int mItemTotalLen{ 0 };
	std::vector < std::shared_ptr<View> > mVisibleItems;
	//std::vector<std::weak_ptr<Node>> mVisibleNodes;//will be added or deleted item frequently
};
#endif
