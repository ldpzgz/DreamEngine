#include"NodeListAdapter.h"
#include "../ListView.h"
#include "../../Node.h"
#include "../UiManager.h"
#include "../TextView.h"
#include "../../MeshFilledRect.h"

void NodeListAdapter::setNode(std::shared_ptr<Node>& pNode) {
	if (pNode) {
		//设置节点进来的时候，只显示根节点
		mpNode = pNode;
		pNode->addAny(static_cast<int>(NodeAnyIndex::UiTreeIndex), std::any(false));
		getAllVisibleItems();
		if (!mpParentView.expired()) {
			auto pParentView = std::dynamic_pointer_cast<ListView>(mpParentView.lock());
			pParentView->getVisibleItems();
		}
	}
}

std::shared_ptr<View>& NodeListAdapter::getView(int position){
	if (position >= mVisibleCount) {
		return gpViewNothing;
	}
	else {
		if (!mpItemView) {
			auto pParent = mpParentView.lock();
			if (pParent) {
				auto& parentRect = pParent->getRect();
				mpItemView = UiManager::loadFromFile("./opengles3/resource/layout/lvItem.xml", parentRect.width, parentRect.height);
				if (mpItemView) {
					mpIconView = mpItemView->findViewById("icon");
					mpTvView = std::dynamic_pointer_cast<TextView>(mpItemView->findViewById("tv"));
				}
			}
		}
		
		if (mpTvView) {
			mpTvView->setText("noName");
		}
		
		return mpItemView;
	}
}

size_t NodeListAdapter::getItemCounts() {
	return mVisibleCount;
}

void NodeListAdapter::getAllVisibleItems() {
	mVisibleNodes.clear();
	if (!mpNode.expired()) {
		auto pRootNode = mpNode.lock();
		if (pRootNode) {
			bool bOver = false;
			pRootNode->visitNode([this](Node* pNode,bool& bVisitChild)->bool {
				if (pNode) {
					++mVisibleCount;
					mVisibleNodes.emplace_back(pNode->shared_from_this());
					auto& any = pNode->getAny(static_cast<int>(NodeAnyIndex::UiTreeIndex));
					if (any.has_value()) {
						try {
							if (!std::any_cast<bool>(any)) {
								bVisitChild = false;
							}
						}
						catch (std::bad_any_cast e) {

						}
					}
					else {
						bVisitChild = false;
					}
				}
				return false;
			}, bOver);
		}
	}
}