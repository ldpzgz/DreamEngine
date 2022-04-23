#include"ListAdapter.h"
#include "../View.h"
#include "../TextView.h"
#include "../UiManager.h"
#include "../../MeshFilledRect.h"

std::shared_ptr<View>& ListAdapter::getView(int position){
	if (position >= mDatas.size()) {
		return gpViewNothing;
	}
	else {
		if (!mpItemView) {
			auto pParent = mpParentView.lock();
			if (pParent) {
				auto& parentRect = pParent->getRect();
				mpItemView = UiManager::loadFromFile("./opengles3/resource/layout/lvItem.xml", parentRect.width, parentRect.height);
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

size_t ListAdapter::getItemCounts() {
	return mDatas.size();
}