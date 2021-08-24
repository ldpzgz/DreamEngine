#include "UiTree.h"
#include "UiRender.h"
#include "../Log.h"
using namespace std;

void UiTree::draw() {
	if (!mViewsToBeDrawing.empty()) {
		//渲染到纹理
		mFbo.setDepthTest(false);
		if (mbRedraw) {
			mFbo.setClearColorValue(0.0f, 0.0f, 0.0f, 0.0f);
			mFbo.setClearColor(true);
			mbRedraw = false;
		}
		else {
			mFbo.setClearColor(false);
		}
		mFbo.startRender();
		for (auto& pView : mViewsToBeDrawing) {
			auto pV = pView.lock();
			if (pV) {
				pV->draw();
			}
		}
		mFbo.endRender();
		mViewsToBeDrawing.clear();
	}
}

void UiTree::addDirtyView(const shared_ptr<View>& pView) {
	mViewsToBeDrawing.emplace_back(pView);
}
void UiTree::updateWidthHeight(float width, float height) {
	mFbo.detachColorTexture();
	if (!mpTexture) {
		mpTexture = make_shared<Texture>();
	}
	mpTexture->unload();
	mpTexture->load(width, height, nullptr, GL_RGBA);
	mFbo.attachColorTexture(mpTexture, 0);
	//ui重绘
	mbRedraw = true;
	if (mpRootView) {
		mpRootView->setDirty(true);
	}
}

void UiTree::calcViewsRect(int windowWidth, int windowHeight) {
	if (mpRootView) {
		//先计算出各个控件的宽高，再计算他们的位置
		calcViewsWidthHeight(windowWidth, windowHeight, mpRootView);
		//rootView的xy默认都是0，0
		calcViewsPos(mpRootView);
	}
}

void UiTree::calcViewsPos(shared_ptr<View> pView) {
	pView->calcChildPos();
	auto& children = pView->getChildren();
	if (!children.empty()) {
		for (auto& pChild : children) {
			if (pChild) {
				calcViewsPos(pChild);
			}
		}
	}
}

void UiTree::calcViewsWidthHeight(int parentWidth, int parentHeight, shared_ptr<View> pView) {
	//计算pView的宽度，以及pView的子view的宽度（如果子view是按百分比布局的)
	if (pView) {
		//如果pView不是按百分比布局的，否则父view已经帮它计算好了
		if (pView->mWidthPercent == 0) {
			pView->calcWidth(parentWidth);
		}
		if (pView->mHeightPercent == 0) {
			pView->calcHeight(parentHeight);
		}

		int myWidth = pView->mRect.width;
		int myHeight = pView->mRect.height;

		//如果子view是按百分比布局的,帮它们算出宽度
		auto totalWPercent = pView->getTotalWidthPercent();
		auto totalHPercent = pView->getTotalHeightPercent();

		for (auto& pChild : pView->mChildren) {
			if (totalWPercent > 0) {
				int childWidth = (int)((float)myWidth*(float)pView->mWidthPercent / (float)totalWPercent);
				pChild->setWidth(childWidth);
			}
			if (totalHPercent > 0) {
				int childHeight = (int)((float)myHeight*(float)pView->mHeightPercent / (float)totalHPercent);
				pChild->setHeight(childHeight);
			}
			calcViewsWidthHeight(myWidth, myHeight, pChild);
		}
	}
}