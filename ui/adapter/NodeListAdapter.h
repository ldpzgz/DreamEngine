#ifndef _NODE_LIST_ADAPTER_H_
#define _NODE_LIST_ADAPTER_H_
#include<string>
#include<list>
#include "../ViewAdapter.h"
class Node;
class TextView;
class NodeListAdapter :public ViewAdapter {
public:
	using ViewAdapter::ViewAdapter;

	void setNode(std::shared_ptr<Node>& pNode);

	std::shared_ptr<View>& getView(int position) override;

	size_t getItemCounts() override;
private:
	void getAllVisibleItems();
	std::shared_ptr<View> mpItemView;
	std::shared_ptr<View> mpIconView;
	std::shared_ptr<TextView> mpTvView;

	std::weak_ptr<Node> mpNode;
	int mVisibleCount{ 0 };
	std::list<std::weak_ptr<Node>> mVisibleNodes;//will be added or deleted item frequently
};
#endif
