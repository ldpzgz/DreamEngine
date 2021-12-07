#ifndef _LIST_ADAPTER_H_
#define _LIST_ADAPTER_H_
#include "../ViewAdapter.h"
class ListAdapter :public ViewAdapter {
public:
	using ViewAdapter::ViewAdapter;

	std::shared_ptr<View>& getView(int position) override;

	int getItemCounts() override;
private:
	std::shared_ptr<View> mpItemView;
	std::vector<std::string> mDatas{ "a","b","c","d","e","f","g","hfjal","hdhf","hakdf","dfadf"};
};
#endif
