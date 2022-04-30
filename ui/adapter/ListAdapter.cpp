#include"ListAdapter.h"
#include "../View.h"
#include "../TextView.h"
#include "../UiManager.h"
#include "../../MeshFilledRect.h"
#include <iostream>

ListAdapter::ListAdapter(std::shared_ptr<View>& pContainerView):
	ViewAdapter(pContainerView)
{

}

void ListAdapter::setData(std::vector<std::string>&& data) {
	mDatas = std::move(data);
	if (!mpItemView) {
		initItemView();
		getVisibleItems();
	}
}

std::shared_ptr<View> ListAdapter::getView(int position){
	int realPos = mFirstVisibleItem + position;
	std::shared_ptr<View> pView;
	if (realPos < mDatas.size() && realPos >= 0) {
		if (mpItemView) {
			if (mVisibleViews.size() < position + 1) {
				auto pv = mpItemView->clone(mpParentView.lock());
				if (pv) {
					mVisibleViews.emplace_back(pv);
					pView = pv;
				}
				else {
					LOGE("clone view error in ListAdapter::getView");
				}
			}
			else {
				pView = mVisibleViews[position];
			}
		}
		if (pView) {
			pView->setAny("text"sv, std::any(mDatas[realPos]));
			auto& pTv = dynamic_pointer_cast<TextView>(pView->findViewById("tv"));
			if (pTv) {
				pTv->setText(mDatas[realPos]);
			}
		}
	}
	return pView;
}

void ListAdapter::initItemView() {
	if (!mpItemView) {
		auto pParent = mpParentView.lock();
		if (pParent) {
			auto& parentRect = pParent->getRect();
			mpItemView = UiManager::loadFromFile("./opengles3/resource/layout/lvItem.xml", parentRect.width, parentRect.height);
			if (mpItemView) {
				mpItemView->setParent(mpParentView);
				mVisibleViews.emplace_back(mpItemView);
				mpItemView->setOnClickListener([](View* pView) {
					auto t = pView->getAny("text");
					if (t.has_value()) {
						std::cout << "list view item text is " << std::any_cast<std::string>(t)<< std::endl;
					}
					});
			}
		}
	}
}

Rect<int>* ListAdapter::getViewRect(int position) {
	int realPos = position;
	if (realPos < mDatas.size() && realPos >= 0) {
		if (mpItemView) {
			return &mpItemView->getRect();
		}
	}
	return nullptr;
}

size_t ListAdapter::getItemCounts() {
	return mDatas.size();
}

bool ListAdapter::getVisibleItems(bool fromFirstItem) {
	if (mDatas.empty()) {
		return false;
	}
	auto pParent = mpParentView.lock();
	if (!pParent) {
		return false;
	}
	int parentTotalLength{ 0 };
	int rectIndex{ 2 };
	auto& pRect = pParent->getRect();
	if (pParent->getOrientation() == LayoutParam::Horizontal) {
		parentTotalLength = pRect.width;
		rectIndex = 2;
	}
	else {
		parentTotalLength = pRect.height;
		rectIndex = 3;
	}
	auto itemCount = static_cast<int>(mDatas.size());
	if (mFirstVisibleItem >= itemCount) {
		mFirstVisibleItem = mLastVisibleItem = itemCount - 1;
		mFirstItemHideLength = mLastItemHideLength = 0;
		fromFirstItem = false;
	}

	int mVisibleItemTotalLength = 0;
	if (fromFirstItem) {
		auto i = mFirstVisibleItem;
		auto pFirstRect = getViewRect(i);
		if (pFirstRect != nullptr) {
			mVisibleItemTotalLength += ((*pFirstRect)[rectIndex]-mFirstItemHideLength);
		}
		while (mVisibleItemTotalLength < parentTotalLength) {
			auto pItemRect = getViewRect(++i);
			if (pItemRect == nullptr) {
				mLastVisibleItem = --i;
				mLastItemHideLength = 0;
				mFirstItemHideLength = mFirstItemHideLength + mVisibleItemTotalLength - parentTotalLength;
				if (mFirstItemHideLength < 0) {
					mFirstItemHideLength = 0;
				}
				return false;
			}
			else {
				mVisibleItemTotalLength += (*pItemRect)[rectIndex];
			}
		}
		if (mVisibleItemTotalLength >= parentTotalLength) {
			mLastVisibleItem = i;
			mLastItemHideLength = mVisibleItemTotalLength - parentTotalLength;
		}
	}
	else {
		auto i = mLastVisibleItem;
		auto pLastRect = getViewRect(i);
		if (pLastRect != nullptr) {
			mVisibleItemTotalLength += ((*pLastRect)[rectIndex] - mLastItemHideLength);
		}
		while (mVisibleItemTotalLength < parentTotalLength) {
			auto pItemRect = getViewRect(--i);
			if (pItemRect == nullptr) {
				mFirstVisibleItem = ++i;
				mFirstItemHideLength = 0;
				mLastItemHideLength = mLastItemHideLength + mVisibleItemTotalLength - parentTotalLength;
				if (mLastItemHideLength < 0) {
					mLastItemHideLength = 0;
				}
				return false;
			}
			else {
				mVisibleItemTotalLength += (*pItemRect)[rectIndex];
			}
		}
		if (mVisibleItemTotalLength >= parentTotalLength) {
			mFirstVisibleItem = i;
			mFirstItemHideLength = mVisibleItemTotalLength - parentTotalLength;
		}
	}
	return true;
}

bool ListAdapter::tryToMove(int moveDistance,bool isVertical) {
	//在这里处理上下左右拖动的逻辑
	//根据移动距离，当前第一个和最后一个可见item，
	//计算出最新的第一个和最后一个可见item
	int moveLength = moveDistance;
	int rectIndex = 2;
	if (isVertical) {
		rectIndex = 3;
	}
	int itemCounts = getVisibleItemCount();
	if (itemCounts > 0) {
		//auto pFirstVisibleItem = getViewRect(mFirstVisibleItem);
		auto pNextItemRect = getViewRect(mLastVisibleItem);
		if (moveLength < 0) {
			//往上拉，或者往左拉，能拉动多少？
			//确定最后一个可见的item
			int realMovedLen = 0;
			int toMove = mLastItemHideLength;
			while (true) {
				int delta = toMove + realMovedLen + moveLength;
				if (delta >= 0) {
					//移动完成
					realMovedLen += (toMove - delta);
					mLastItemHideLength = delta;
					break;
				}
				else {
					//继续移动
					realMovedLen += toMove;
					pNextItemRect = getViewRect(++mLastVisibleItem);
					if (pNextItemRect == nullptr) {
						--mLastVisibleItem;
						mLastItemHideLength = 0;
						break;
					}
					else {
						toMove = (*pNextItemRect)[rectIndex];
					}
				}
			}
			if (realMovedLen == 0) {
				//拉不动了
				return false;
			}
			getVisibleItems(false);
			return true;
		}
		else if (moveLength > 0) {
			//往下拉，或者往右拉，能拉多少？
			//确定第一个可见的item
			int realMovedLen = 0;
			int toMove = mFirstItemHideLength;
			while (true) {
				int delta = toMove + realMovedLen - moveLength;
				if (delta >= 0) {
					//移动完成
					realMovedLen += (toMove - delta);
					mFirstItemHideLength = delta;
					break;
				}
				else {
					//继续移动
					realMovedLen += toMove;
					pNextItemRect = getViewRect(--mFirstVisibleItem);
					if (pNextItemRect == nullptr) {
						++mFirstVisibleItem;
						mFirstItemHideLength = 0;
						break;
					}
					else {
						toMove = (*pNextItemRect)[rectIndex];
					}
				}
			}
			if (realMovedLen == 0) {
				//拉不动了
				return false;
			}
			getVisibleItems();
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return false;
	}
	return true;
}

void ListAdapter::forVisibleItems(const std::function<bool(std::shared_ptr<View>&)>& func) {
	for (auto& pView : mVisibleViews) {
		if (pView && func) {
			if (func(pView)){
				break;
			}
		}
	}
}