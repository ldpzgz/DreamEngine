#ifndef _VIEW_ADAPTER_H_
#define _VIEW_ADAPTER_H_

#include "View.h"
class ViewAdapter
{
public:
	ViewAdapter() = default;
	virtual ~ViewAdapter() {

	}

	virtual std::shared_ptr<View>& getView(int position) = 0;

	virtual int getItemCounts() {
		return 0;
	}
};
#endif