#include "ScrollView.h"
#include "UiRender.h"
#include "core/MeshFilledRect.h"

std::shared_ptr<View> ScrollView::clone() {
	auto pParent = std::make_shared<ScrollView>(*this);
	for (const auto& pChild : mChildren) {
		auto pNewChild = pChild->clone();
		if (pNewChild) {
			pNewChild->setParent(pParent);
			pParent->addChild(pNewChild);
		}
	}
	return pParent;
}

void ScrollView::draw() {
	UiRender::getInstance()->drawScrollView(this);
	View::draw();
}

bool ScrollView::mouseMove(int x, int y,bool notInside) {
	if (!notInside && (mMouseState & MouseState::MouseLButtonDown) && mRect.isInside(x, y)) {
		mMouseState |= MouseState::MouseHover;
		//在这里处理上下左右拖动的逻辑
		/*
		* 只看第一个和最后一个子view是否到了不可移动的位置，否则就translate相应的距离。
		*/
		glm::ivec2 temp{0,0};
		if (isVertical()) {
			temp.y = y - mPrePos.y;
			if (!mChildren.empty()) {
				auto& pFrontChild = mChildren.front();
				auto& pEndChild = mChildren.back();
				if (temp.y < 0) {
					auto childRect = pEndChild->getRect();
					const auto& childMove = pEndChild->getTranslateVector();
					childRect.translate(childMove);
					auto pBottom = mRect.y + mRect.height;
					auto cBottom = childRect.y + childRect.height;
					if (cBottom <= pBottom) {
						/*
						* 已经往上拖到底了
						*/
						return false;
					}
					else {
						auto delta = pBottom - cBottom;
						if (temp.y < delta) {
							temp.y = delta;
						}
					}
				}
				else if (temp.y > 0) {
					auto childRect = pFrontChild->getRect();
					auto childMove = pFrontChild->getTranslateVector();
					childRect.translate(childMove);
					if (childRect.y >= mRect.y) {
						/*
						* 已经往下拖到底了
						*/
						return false;
					}
					else {
						auto delta = mRect.y - childRect.y;
						if (temp.y > delta) {
							temp.y = delta;
						}
					}
				}
				else {
					return false;
				}
			}
		}
		else {
			temp.x = x - mPrePos.x;
			if (!mChildren.empty()) {
				auto& pFrontChild = mChildren.front();
				auto& pEndChild = mChildren.back();
				if (temp.x < 0) {
					auto childRect = pEndChild->getRect();
					const auto& childMove = pEndChild->getTranslateVector();
					childRect.translate(childMove);
					auto cRight = childRect.x + childRect.width;
					auto pRight = mRect.x + mRect.width;
					if (cRight <= pRight) {
						/*
						* 已经往上拖到底了
						*/
						return false;
					}
					else {
						auto delta = pRight - cRight;
						if (temp.x < delta) {
							temp.x = delta;
						}
					}
				}
				else if (temp.x > 0) {
					auto childRect = pFrontChild->getRect();
					const auto& childMove = pFrontChild->getTranslateVector();
					childRect.translate(childMove);
					if (childRect.x >= mRect.x) {
						/*
						* 已经往下拖到底了
						*/
						return false;
					}
					else {
						auto delta = mRect.x - childRect.x;
						if (temp.x > delta) {
							temp.x = delta;
						}
					}
				}
				else {
					return false;
				}
			}
		}
		translate(temp);
		setDirty(true);
		mPrePos.x = x;
		mPrePos.y = y;
		return true;
	}
	return false;
}

bool ScrollView::mouseLButtonDown(int x, int y, bool notInside) {
	if (!notInside && mRect.isInside(x, y)) {
		mMouseState |= MouseState::MouseLButtonDown;
		mbLButtonDown = true;
		mPrePos.x = x;
		mPrePos.y = y;
		return true;
	}
	return false;
}

bool ScrollView::mouseLButtonUp(int x, int y,bool notInside) {
	if (mMouseState & MouseState::MouseLButtonDown) {
		mMouseState = MouseState::MouseNone;
		//setDirty(true);
		return true;
	}
	return false;
}

void ScrollView::afterGetWidthHeight() {
	auto pthis = dynamic_pointer_cast<ScrollView>(shared_from_this());
	if (isVertical()) {
		for (auto& pChild : mChildren) {
			mChildTotalWidthHeight.y += pChild->advanceY();
			pChild->setMoveListener(std::static_pointer_cast<MoveListener>(pthis));
		}
	}
	else {
		for (auto& pChild : mChildren) {
			mChildTotalWidthHeight.x += pChild->advanceX();
			pChild->setMoveListener(std::static_pointer_cast<MoveListener>(pthis));
		}
	}
}
