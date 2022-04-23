#include "ListView.h"
#include "UiRender.h"
#include "../Log.h"
#include "../MeshFilledRect.h"

void ListView::setAdapter(const std::shared_ptr<ViewAdapter>& pAdapter) {
	mpAdapter = pAdapter;
	mFirstVisibleItem = 0;
	mFirstItemHideLength = 0;
	getVisibleItems();
}

void ListView::getVisibleItems() {
	if (mFirstVisibleItem >= mpAdapter->getItemCounts()) {
		mFirstVisibleItem = mLastVisibleItem = static_cast<int>(mpAdapter->getItemCounts()) - 1;
		mFirstItemHideLength = mLastItemHideLength = 0;
	}

	mVisibleItemTotalLength = 0;
	int rectIndex = 2;
	if (mOrientation == LayoutParam::Vertical) {
		rectIndex = 3;
	}
	//从第一个开始往后数
	auto i = mFirstVisibleItem;
	mLastVisibleItem = mFirstVisibleItem;
	auto pItem = mpAdapter->getView(i);
	mVisibleItemTotalLength += pItem->getRect()[rectIndex] - mFirstItemHideLength;
	do {
		if (mVisibleItemTotalLength >= mRect[rectIndex]) {
			mLastItemHideLength = mVisibleItemTotalLength - mRect[rectIndex];
			break;
		}
		pItem = mpAdapter->getView(++i);
		if (pItem) {
			mVisibleItemTotalLength += pItem->getRect()[rectIndex];
			mLastVisibleItem = i;
		}
		else {
			//找到了最后一个item都还没有满足，就从之前的firstItem开始往前确定第一个可见item
			int j = mFirstVisibleItem;
			mLastItemHideLength = 0;
			mVisibleItemTotalLength += mFirstItemHideLength;
			mFirstItemHideLength = 0;
			do {
				if (mVisibleItemTotalLength >= mRect[rectIndex]) {
					mFirstItemHideLength = mVisibleItemTotalLength - mRect[rectIndex];
					break;
				}
				else {
					pItem = mpAdapter->getView(--j);
					if (pItem) {
						mFirstVisibleItem = j;
						mVisibleItemTotalLength += pItem->getRect()[rectIndex];
					}
				}
			} while (pItem);
			break;
		}
	} while (pItem);

	if (mVisibleItemTotalLength >= mRect[rectIndex]) {
		mbMovable = true;
	}
}

void ListView::draw() {
	UiRender::getInstance()->drawListView(this);
	View::draw();
}

bool ListView::tryToMove(int moveDistance) {
	//在这里处理上下左右拖动的逻辑
	//根据移动距离，当前第一个和最后一个可见item，
	//计算出最新的第一个和最后一个可见item
	int moveLength = moveDistance;
	int rectIndex = 2;
	if (mOrientation == LayoutParam::Vertical) {
		rectIndex = 3;
	}

	int itemCounts = static_cast<int>(mpAdapter->getItemCounts());
	if (itemCounts > 0) {
		auto pFirstVisibleItem = mpAdapter->getView(mFirstVisibleItem);
		auto pLastVisibleItem = mpAdapter->getView(mLastVisibleItem);
		if (moveLength < 0) {
			//往上拉，或者往左拉，能拉动多少？
			//确定最后一个可见的item
			int tempLength = moveLength;
			while (mLastItemHideLength + moveLength < 0) {
				moveLength += mLastItemHideLength;
				pLastVisibleItem = mpAdapter->getView(++mLastVisibleItem);
				if (!pLastVisibleItem) {
					--mLastVisibleItem;
					break;
				}
				mLastItemHideLength = pLastVisibleItem->getRect()[rectIndex];
			}
			if (!pLastVisibleItem) {
				mLastItemHideLength = 0;
				tempLength -= moveLength;
			}
			else {
				mLastItemHideLength += moveLength;
			}
			//确定第一个可见的item
			moveLength = tempLength;
			if (moveLength == 0) {
				//拉不动了
				return false;
			}
			do {
				int firstLength = pFirstVisibleItem->getRect()[rectIndex];
				if (firstLength - mFirstItemHideLength + moveLength < 0) {
					pFirstVisibleItem = mpAdapter->getView(++mFirstVisibleItem);
					moveLength += (firstLength - mFirstItemHideLength);
					mFirstItemHideLength = 0;
				}
				else {
					mFirstItemHideLength -= moveLength;
					break;
				}

			} while (true);
		}
		else if (moveLength > 0) {
			//往下拉，或者往右拉，能拉多少？
			//确定第一个可见的item
			int tempLength = moveLength;
			while (moveLength - mFirstItemHideLength > 0) {
				moveLength -= mFirstItemHideLength;
				if (mFirstVisibleItem > 0) {
					pFirstVisibleItem = mpAdapter->getView(--mFirstVisibleItem);
					mFirstItemHideLength = pFirstVisibleItem->getRect()[rectIndex];
				}
				else {
					mFirstItemHideLength = 0;
					tempLength -= moveLength;
					moveLength = 0;
					break;
				}
			}
			mFirstItemHideLength -= moveLength;
			//确定最后一个可见的item
			moveLength = tempLength;
			if (moveLength == 0) {
				//拉不动了
				return false;
			}
			do {
				int height = pLastVisibleItem->getRect()[rectIndex];
				if (moveLength - (height - mLastItemHideLength) > 0) {
					moveLength -= (height - mLastItemHideLength);
					pLastVisibleItem = mpAdapter->getView(--mLastVisibleItem);
					mLastItemHideLength = 0;
				}
				else {
					mLastItemHideLength += moveLength;
					break;
				}

			} while (true);
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

bool ListView::mouseMove(int x, int y, bool notInside) {
	if (!mpAdapter || !mbMovable) {
		return true;
	}
	if (mMouseState & MouseState::MouseLButtonDown) {
		mMouseState |= MouseState::MouseHover;
		int moveDistance = 0;
		if (mOrientation == LayoutParam::Vertical) {
			moveDistance = y - mPrePos.y;
		}
		else {
			moveDistance = x - mPrePos.x;
		}
		if (tryToMove(moveDistance)) {
			setDirty(true);
			mPrePos.x = x;
			mPrePos.y = y;
			return true;
		}
		else {
			return false;
		}
	}
	return false;
}

bool ListView::mouseLButtonDown(int x, int y, bool notInside) {
	if (!notInside && mRect.isInside(x, y)) {
		mMouseState |= MouseState::MouseLButtonDown;
		mbLButtonDown = true;
		mPrePos.x = x;
		mPrePos.y = y;
		return true;
	}
	return false;
}

bool ListView::mouseLButtonUp(int x, int y,bool notInside) {
	if (mMouseState & MouseState::MouseLButtonDown) {
		mMouseState = MouseState::MouseNone;
		//setDirty(true);
		return true;
	}
	return false;
}

void ListView::orientationHandler(const string& value) {
	if (value == "h") {
		mOrientation = LayoutParam::Horizontal;
	}
	else if (value == "v") {
		mOrientation = LayoutParam::Vertical;
	}
	else {
		LOGE("can not recognize linearlayout orientation value %s", value.c_str());
	}
}