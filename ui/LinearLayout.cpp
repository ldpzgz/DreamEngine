#include "LinearLayout.h"
#include "UiRender.h"

int LinearLayout::getTotalWidthPercent() {
	if (!mChildren.empty()) {
		for (auto& child : mChildren) {
			if (child) {
				mTotalWidthPercent += child->mWidthPercent;
			}
		}
	}
	return mTotalWidthPercent;
}

int LinearLayout::getTotalHeightPercent() {
	if (!mChildren.empty()) {
		for (auto& child : mChildren) {
			if (child) {
				mTotalHeightPercent += child->mHeightPercent;
			}
		}
	}
	return mTotalHeightPercent;
}

void LinearLayout::calcChildPos() {
	int x = mRect.x;
	int y = mRect.y;
	int totalWidth = 0;
	int totalHeight = 0;
	
	for (auto& pChild : mChildren) {
		if (mOrientation == LayoutParam::Horizontal) {
			totalWidth += pChild->advanceX();
		}
		else if (mOrientation == LayoutParam::Vertical) {
			totalHeight += pChild->advanceY();
		}
	}

	if (mOrientation == LayoutParam::Horizontal) {
		if (mGravity == LayoutParam::LeftTop) {
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->calcX(x);
					pChild->calcY(y);
					x += pChild->advanceX();
				}
			}
		}
		else if (mGravity == LayoutParam::TopCenter) {
			x = mRect.x + (mRect.width - totalWidth) / 2;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->calcX(x);
					pChild->calcY(y);
					x += pChild->advanceX();
				}
			}
		}
		else if (mGravity == LayoutParam::LeftCenter) {
			y = mRect.y + mRect.height / 2;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->calcX(x);
					pChild->setCenterY(y);
					x += pChild->advanceX();
				}
			}
		}
		else if (mGravity = LayoutParam::Center) {
			x = mRect.x + (mRect.width - totalWidth) / 2;
			y = mRect.y + mRect.height / 2;

			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->calcX(x);
					pChild->setCenterY(y);
					x += pChild->advanceX();
				}
			}
		}
	}
	if (mOrientation == LayoutParam::Vertical) {
		if (mGravity == LayoutParam::LeftTop) {
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->calcX(x);
					pChild->calcY(y);
					y += pChild->advanceY();
				}
			}
		}
		else if (mGravity == LayoutParam::TopCenter) {
			x = mRect.x + mRect.width/ 2;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->setCenterX(x);
					pChild->calcY(y);
					y += pChild->advanceY();
				}
			}
		}
		else if (mGravity == LayoutParam::LeftCenter) {
			y = mRect.y + (mRect.height-totalHeight) / 2;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->calcX(x);
					pChild->calcY(y);
					y += pChild->advanceY();
				}
			}
		}
		else if (mGravity = LayoutParam::Center) {
			x = mRect.x + mRect.width / 2;
			y = mRect.y + (mRect.height - totalHeight) / 2;

			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->setCenterX(x);
					pChild->calcY(y);
					y += pChild->advanceY();
				}
			}
		}
	}
}

void LinearLayout::draw() {
	UiRender::getInstance()->drawLinearLayout(this);
	View::draw();
}

void LinearLayout::orientationHandler(const shared_ptr<View>& pv, const string& value) {
	auto pl = dynamic_pointer_cast<LinearLayout>(pv);
	if (pl) {
		if (value == "h") {
			pl->mOrientation = LayoutParam::Horizontal;
		}
		else if (value == "v") {
			pl->mOrientation = LayoutParam::Vertical;
		}
		else {
			LOGE("can not recognize orientation value %s",value.c_str());
		}
	}
}