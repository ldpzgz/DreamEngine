#include "ListView.h"
#include "UiRender.h"
#include "core/Log.h"
#include "core/MeshFilledRect.h"

std::shared_ptr<View> ListView::clone() {
	return std::make_shared<ListView>(*this);
}

void ListView::setAdapter(const std::shared_ptr<ViewAdapter>& pAdapter) {
	mpAdapter = pAdapter;
	mFirstVisibleItem = 0;
	mFirstItemHideLength = 0;
	if (mpAdapter) {
		int totalLen = 0;
		if (mOrientation == LayoutParam::Vertical) {
			totalLen = mRect[3];
		}
		else {
			totalLen = mRect[2];
		}
		mbMovable = mpAdapter->getVisibleItems();
	}
}

int ListView::getVisibleItemCount() {
	if (mpAdapter) {
		return mpAdapter->getVisibleItemCount();
	}
	return 0;
}

void ListView::draw() {
	UiRender::getInstance()->drawListView(this);
	View::draw();
}

bool ListView::mouseMove(int x, int y, bool notInside) {
	if (!mpAdapter) {
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
		if (mpAdapter->tryToMove(moveDistance, mOrientation == LayoutParam::Vertical)) {
			setDirty(true);
			mPrePos.x = x;
			mPrePos.y = y;
			return true;
		}
		else {
			return false;
		}
	}
	else {
		return View::mouseMove(x, y, notInside);
	}
	return false;
}

bool ListView::mouseLButtonDown(int x, int y, bool notInside) {
	auto tempRect = mRect;
	tempRect.x += mMoveVector.x;
	tempRect.y += mMoveVector.y;
	if (!notInside && tempRect.isInside(x, y)) {
		mPrePos.x = x;
		mPrePos.y = y;
		if (mpAdapter) {
			mpAdapter->forVisibleItems([x, y, notInside](std::shared_ptr<View>& pView)->bool {
				return pView->mouseLButtonDown(x, y, notInside);
				});
		}
	}
	
	return View::mouseLButtonDown(x,y,notInside);
}

bool ListView::mouseLButtonUp(int x, int y,bool notInside) {
	if (mpAdapter) {
		auto tempRect = mRect;
		tempRect.x += mMoveVector.x;
		tempRect.y += mMoveVector.y;
		if (!notInside && tempRect.isInside(x, y)) {
			mpAdapter->forVisibleItems([x, y, notInside](std::shared_ptr<View>& pView)->bool {
				return pView->mouseLButtonUp(x, y, notInside);
				});
		}
		else {
			mpAdapter->forVisibleItems([x, y, notInside](std::shared_ptr<View>& pView)->bool {
				return pView->mouseLButtonUp(x, y, true);
			});
		}
	}
	return View::mouseLButtonUp(x, y, notInside);
	
}

