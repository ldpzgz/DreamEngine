#ifndef _VIEW_ADAPTER_H_
#define _VIEW_ADAPTER_H_

#include "View.h"
class ViewAdapter
{
public:
	ViewAdapter(std::shared_ptr<View>& pContainerView) :
		mpParentView(pContainerView)
	{

	}
	ViewAdapter() = default;
	virtual ~ViewAdapter() {

	}

	virtual std::shared_ptr<View>& getView(int position) = 0;

	virtual size_t getItemCounts() {
		return 0;
	}
protected:
	std::weak_ptr<View> mpParentView;
};
#endif