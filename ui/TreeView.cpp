#include "TreeView.h"
#include "UiRender.h"
#include "core/Log.h"
#include "core/MeshFilledRect.h"

TreeView::TreeView() {
	mOrientation = LayoutParam::Vertical;
}

TreeView::TreeView(const std::shared_ptr<View> pC):
	View(pC)
{
	mOrientation = LayoutParam::Vertical;
}


std::shared_ptr<View> TreeView::clone() {
	return std::make_shared<TreeView>(*this);
}

void TreeView::setAdapter(const std::shared_ptr<ViewAdapter>& pAdapter) {
	mpAdapter = pAdapter;
	if (mpAdapter) {
		mpAdapter->getVisibleItems();
	}
}

int TreeView::getVisibleItemCount() {
	if (mpAdapter) {
		return mpAdapter->getVisibleItemCount();
	}
	return 0;
}

void TreeView::draw() {
	UiRender::getInstance()->drawTreeView(this);
	View::draw();
}

bool TreeView::tryToMove(int moveDistance) {
	return false;
}

bool TreeView::mouseMove(int x, int y, bool notInside) {
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
		if (moveDistance!=0 && mpAdapter->tryToMove(moveDistance, true)) {
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

bool TreeView::mouseLButtonDown(int x, int y, bool notInside) {
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

	return View::mouseLButtonDown(x, y, notInside);
}

bool TreeView::mouseLButtonUp(int x, int y,bool notInside) {
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

void TreeView::orientationHandler(const string& value) {
	mOrientation = LayoutParam::Vertical;
	LOGD("the orientation of TreeView is allways vertical");
}