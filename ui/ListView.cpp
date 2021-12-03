#include "ListView.h"
#include "UiRender.h"

void ListView::setAdapter(const std::shared_ptr<ViewAdapter>& pAdapter) {
	mpAdapter = pAdapter;
	mFirstVisibleItem = 0;
	mFirstItemHideLength = 0;
	getVisibleItems();
}

void ListView::getVisibleItems() {
	if (mFirstVisibleItem >= mpAdapter->getItemCounts()) {
		mFirstVisibleItem = mLastVisibleItem = mpAdapter->getItemCounts() - 1;
		mFirstItemHideLength = mLastItemHideLength = 0;
	}

	mVisibleItemTotalLength = 0;
	int rectIndex = 2;
	if (mOrientation == LayoutParam::Vertical) {
		rectIndex = 3;
	}
	//�ӵ�һ����ʼ������
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
			//�ҵ������һ��item����û�����㣬�ʹ�֮ǰ��firstItem��ʼ��ǰȷ����һ���ɼ�item
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
}

//void ListView::getVisibleItems() {
//	if (mpAdapter) {
//		//�ҵ����һ���ɼ���item
//		mLastVisibleItem = mFirstVisibleItem;
//		auto pView = mpAdapter->getView(mFirstVisibleItem);
//		mVisibleItemTotalLength = 0;
//		mLastItemHideLength = 0;
//
//		int rectIndex = 2;
//		if (mOrientation == LayoutParam::Vertical) {
//			rectIndex = 3;
//		}
//		while (pView) {
//			mVisibleItemTotalLength += pView->getRect()[rectIndex];
//			if (mVisibleItemTotalLength - mFirstItemHideLength < mRect[rectIndex]) {
//				pView = mpAdapter->getView(++mLastVisibleItem);
//			}
//			else {
//				break;
//			}
//		}
//		auto delta = mVisibleItemTotalLength - mRect[rectIndex];
//		if (delta >= 0) {
//			mLastItemHideLength = delta - mFirstItemHideLength;
//			mbMovable = true;
//		}
//		else {
//			//ȫ��item�������ĳ��ȶ�û��listview�ĳ��ȳ�
//			mLastItemHideLength = 0;
//			mbMovable = false;
//		}
//
//	}
//}

void ListView::draw() {
	UiRender::getInstance()->drawListView(this);
	View::draw();
}

bool ListView::move(int moveDistance) {
	//�����ﴦ�����������϶����߼�
	int moveLength = moveDistance;
	int rectIndex = 2;
	if (mOrientation == LayoutParam::Vertical) {
		rectIndex = 3;
	}

	int itemCounts = mpAdapter->getItemCounts();
	if (itemCounts > 0) {
		auto pFirstVisibleItem = mpAdapter->getView(mFirstVisibleItem);
		auto pLastVisibleItem = mpAdapter->getView(mLastVisibleItem);
		if (moveLength < 0) {
			//�����������������������������٣�
			//ȷ�����һ���ɼ���item
			int tempLength = moveLength;
			while (mLastItemHideLength + moveLength < 0) {
				moveLength += mLastItemHideLength;
				pLastVisibleItem = mpAdapter->getView(++mLastVisibleItem);
				if (!pLastVisibleItem) {
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
			//ȷ����һ���ɼ���item
			moveLength = tempLength;
			if (moveLength == 0) {
				//��������
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
			//���������������������������٣�
			//ȷ����һ���ɼ���item
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
			//ȷ�����һ���ɼ���item
			moveLength = tempLength;
			if (moveLength == 0) {
				//��������
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

bool ListView::mouseMove(int x, int y) {
	if (!mpAdapter || !mbMovable) {
		return true;
	}
	if (mMouseState & MouseState::MouseLButtonDown) {
		mMouseState |= MouseState::MouseOver;
		int moveDistance = 0;
		if (mOrientation == LayoutParam::Vertical) {
			moveDistance = y - mPrePos.y;
		}
		else {
			moveDistance = x - mPrePos.x;
		}
		if (move(moveDistance)) {
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

bool ListView::mouseLButtonDown(int x, int y) {
	if (mRect.isInside(x, y)) {
		mMouseState |= MouseState::MouseLButtonDown;
		mbLButtonDown = true;
		mPrePos.x = x;
		mPrePos.y = y;
		return true;
	}
	return false;
}

bool ListView::mouseLButtonUp(int x, int y) {
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