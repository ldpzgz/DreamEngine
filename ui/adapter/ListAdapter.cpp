#include"ListAdapter.h"
#include "../UiManager.h"

std::shared_ptr<View>& ListAdapter::getView(int position){
	if (position >= mDatas.size()) {
		return gpViewNothing;
	}
	else {
		if (!mpItemView) {
			auto pParent = mpParentView.lock();
			if (pParent) {
				auto& parentRect = pParent->getRect();
				mpItemView = UiManager::loadFromFile("./opengles3/material/layout/lvItem.xml", parentRect.width, parentRect.height);
			}
		}
		if (mpItemView) {
			auto& pTv =dynamic_pointer_cast<TextView>( mpItemView->findViewById("tv"));
			if (pTv) {
				pTv->setText(mDatas[position]);
			}
		}
		return mpItemView;
	}
	
}

int ListAdapter::getItemCounts() {
	return mDatas.size();
}