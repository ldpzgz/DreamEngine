#pragma once

#include "ListAdapter.h"
#include "UiManager.h"
#include <vector>
#include <memory>
class ListViewAdapter :public ListAdapter{
public:
	using ListAdapter::ListAdapter;

	void initItemView() override;

	int getViewTypeCount() override {
		return 2;
	}

	void setViewData(const std::shared_ptr<View>& pView, int type, int dataPos) override;
};

