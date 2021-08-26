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
	int childTotalWidth = 0;
	int childTotalHeight = 0;
	
	for (auto& pChild : mChildren) {
		if (mOrientation == LayoutParam::Horizontal) {
			childTotalWidth += pChild->advanceX();
		}
		else if (mOrientation == LayoutParam::Vertical) {
			childTotalHeight += pChild->advanceY();
		}
	}

	if (mOrientation == LayoutParam::Horizontal) {
		if (mGravity == LayoutParam::LeftTop) {
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->alignLeftX(x);
					pChild->alignTopY(y);
					x += pChild->advanceX();
				}
			}
		}
		else if (mGravity == LayoutParam::LeftBottom) {
			y = mRect.y + mRect.height;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->alignLeftX(x);
					pChild->alignBottomY(y);
					x += pChild->advanceX();
				}
			}
		}
		else if (mGravity == LayoutParam::RightTop) {
			x = mRect.x + mRect.width - childTotalWidth;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->alignLeftX(x);
					pChild->alignTopY(y);
					x += pChild->advanceX();
				}
			}
		}
		else if (mGravity == LayoutParam::RightBottom) {
			x = mRect.x + mRect.width - childTotalWidth;
			y = mRect.y + mRect.height;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->alignLeftX(x);
					pChild->alignBottomY(y);
					x += pChild->advanceX();
				}
			}
		}
		else if (mGravity == LayoutParam::TopCenter) {
			x = mRect.x + (mRect.width - childTotalWidth) / 2;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->alignLeftX(x);
					pChild->alignTopY(y);
					x += pChild->advanceX();
				}
			}
		}
		else if (mGravity == LayoutParam::BottomCenter) {
			x = mRect.x + (mRect.width - childTotalWidth) / 2;
			y = mRect.y + mRect.height;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->alignLeftX(x);
					pChild->alignBottomY(y);
					x += pChild->advanceX();
				}
			}
		}
		else if (mGravity == LayoutParam::LeftCenter) {
			y = mRect.y + mRect.height / 2;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->alignLeftX(x);
					pChild->setCenterY(y);
					x += pChild->advanceX();
				}
			}
		}
		else if (mGravity == LayoutParam::RightCenter) {
			y = mRect.y + mRect.height / 2;
			x = mRect.x + (mRect.width - childTotalWidth);
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->alignLeftX(x);
					pChild->setCenterY(y);
					x += pChild->advanceX();
				}
			}
		}
		else if (mGravity == LayoutParam::Center) {
			x = mRect.x + (mRect.width - childTotalWidth) / 2;
			y = mRect.y + mRect.height / 2;

			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->alignLeftX(x);
					pChild->setCenterY(y);
					x += pChild->advanceX();
				}
			}
		}
	}
	else if (mOrientation == LayoutParam::Vertical) {
		if (mGravity == LayoutParam::LeftTop) {
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->alignLeftX(x);
					pChild->alignTopY(y);
					y += pChild->advanceY();
				}
			}
		}
		else if (mGravity == LayoutParam::LeftBottom) {
			y = mRect.y + mRect.height - childTotalHeight;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->alignLeftX(x);
					pChild->alignTopY(y);
					y += pChild->advanceY();
				}
			}
		}
		else if (mGravity == LayoutParam::RightBottom) {
			x = mRect.x + mRect.width;
			y = mRect.y + mRect.height - childTotalHeight;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->alignRightX(x);
					pChild->alignTopY(y);
					y += pChild->advanceY();
				}
			}
		}
		else if (mGravity == LayoutParam::RightTop) {
			x = mRect.x + mRect.width;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->alignRightX(x);
					pChild->alignTopY(y);
					y += pChild->advanceY();
				}
			}
		}
		else if (mGravity == LayoutParam::TopCenter) {
			x = mRect.x + mRect.width/ 2;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->setCenterX(x);
					pChild->alignTopY(y);
					y += pChild->advanceY();
				}
			}
		}
		else if (mGravity == LayoutParam::LeftCenter) {
			y = mRect.y + (mRect.height-childTotalHeight) / 2;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->alignLeftX(x);
					pChild->alignTopY(y);
					y += pChild->advanceY();
				}
			}
		}
		else if (mGravity == LayoutParam::RightCenter) {
			x = mRect.x + mRect.width;
			y = mRect.y + (mRect.height - childTotalHeight) / 2;
			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->alignRightX(x);
					pChild->alignTopY(y);
					y += pChild->advanceY();
				}
			}
		}
		else if (mGravity == LayoutParam::Center) {
			x = mRect.x + mRect.width / 2;
			y = mRect.y + (mRect.height - childTotalHeight) / 2;

			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->setCenterX(x);
					pChild->alignTopY(y);
					y += pChild->advanceY();
				}
			}
		}
		else if (mGravity == LayoutParam::BottomCenter) {
			x = mRect.x + mRect.width / 2;
			y = mRect.y + (mRect.height - childTotalHeight);

			for (auto& pChild : mChildren) {
				if (pChild) {
					pChild->setCenterX(x);
					pChild->alignTopY(y);
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