#include "ListView.h"
#include "TextView.h"
#include "ListViewAdapter.h"
#include "ListViewData.h"

void ListViewAdapter::initItemView() {
	if (!mItemViews.empty()) {
		return;
	}
	int typeCount = getViewTypeCount();
	mItemViews.resize(typeCount);
	mItemViewRefCount.resize(typeCount);
	auto pParent = mpParentView.lock();
	if (pParent) {
		auto& parentRect = pParent->getRect();
		auto view0 = UiManager::loadFromFile("./opengles3/resource/layout/lvItem.xml",
			parentRect.width, parentRect.height);
		auto view1 = UiManager::loadFromFile("./opengles3/resource/layout/lvItem2.xml",
			parentRect.width, parentRect.height);
		if (view0 && view1) {
			mItemViews[0].emplace_back(view0);
			mItemViews[1].emplace_back(view1);
		}
		else {
			LOGE("error to load listview item view");
		}
	}
}

void ListViewAdapter::setViewData(const std::shared_ptr<View>& pView, int type, int dataPos) {
	if (pView) {
		auto pData = std::dynamic_pointer_cast<ListViewData>(mpDatas);
		auto pTextView = std::dynamic_pointer_cast<TextView>(pView->findViewById("tv"));
		if (pData && pTextView) {
			auto& str = pData->getString(dataPos);
			pTextView->setText(str);
		}
	}
}