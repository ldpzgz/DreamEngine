#include "UiTree.h"
#include "UiRender.h"
#include "../Log.h"
using namespace std;

bool UiTree::draw() {
	if (!mViewsToBeDrawing.empty()) {
		//渲染到纹理
		mFboForRender.setDepthTest(false);
		if (mbRedraw) {
			mFboForRender.setClearColorValue(0.0f, 0.0f, 0.0f, 0.0f);
			mFboForRender.setClearColor(true);
			mbRedraw = false;
		}
		else {
			mFboForRender.setClearColor(false);
		}
		mFboForRender.startRender();
		glEnable(GL_BLEND);
		glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE);
		glBlendEquationSeparate(GL_FUNC_ADD, GL_MAX);
		for (auto& pView : mViewsToBeDrawing) {
			auto pV = pView.lock();
			if (pV) {
				pV->draw();
			}
		}
		glDisable(GL_BLEND);
		mFboForRender.endRender();
		mViewsToBeDrawing.clear();
		return true;
	}
	return false;
}

void UiTree::addDirtyView(const shared_ptr<View>& pView) {
	mViewsToBeDrawing.emplace_back(pView);
}
void UiTree::updateWidthHeight(float width, float height) {
	mFboForRender.detachColorRbo();
	if (!mpTexture) {
		mpTexture = make_shared<Texture>();
	}
	mpTexture->unload();
	mpTexture->load(width, height, nullptr, GL_RGBA);
	UiRender::getInstance()->setTexture(mpTexture);
	mFboForRender.attachColorTexture(mpTexture,0);
	//mFboForRender.attachColorRbo(0, width, height);
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
	//计算pView的宽度高度，以及pView的子view的宽度高度（如果子view是按百分比布局的)
	if (pView) {
		/*
		从顶向下计算各个view的宽度和高度，不能相互依赖
		1 父view是wrapContent，需要根据子view的尺寸来计算出父view的尺寸
		2 子view是percentWidth，或者matchParent，需要根据父view的尺寸来计算子view的尺寸
		3 不能出现父子相互依赖的情况。
		*/
		//matchParent,或者固定尺寸，可计算出来
		bool hasGetWidth = pView->calcWidth(parentWidth);
		bool hasGetHeight = pView->calcHeight(parentHeight);

		int myWidth = pView->mRect.width;
		int myHeight = pView->mRect.height;

		//如果子view是按百分比布局的,帮它们算出宽度
		auto totalWPercent = pView->getChildrenTotalWidthPercent();
		auto totalHPercent = pView->getChildrenTotalHeightPercent();

		//计算子view的宽度，高度
		for (auto& pChild : pView->mChildren) {
			if (totalWPercent > 0) {
				if (!hasGetWidth) {
					//父与子之间不能相互依赖，父是wrapContent，子是百分比布局，这样是不行的
					LOGE("ERROR to calc child's width");
				}
				int childWidth = (int)((float)myWidth*(float)pChild->mWidthPercent / (float)totalWPercent);
				pChild->setWidth(childWidth);
			}
			if (totalHPercent > 0) {
				if (!hasGetHeight) {
					//父与子之间不能相互依赖，父是wrapContent，子是百分比布局，这样是不行的
					LOGE("ERROR to calc child's height");
				}
				int childHeight = (int)((float)myHeight*(float)pChild->mHeightPercent / (float)totalHPercent);
				pChild->setHeight(childHeight);
			}
			//计算子view的尺寸
			calcViewsWidthHeight(myWidth, myHeight, pChild);
		}

		//wrapContent的情况下，根据子view计算出尺寸
		if (!hasGetWidth) {
			pView->getWidthAccordChildren();
		}
		if (!hasGetHeight) {
			pView->getHeightAccordChildren();
		}
		//确定了宽高之后，就可以初始化承载背景的shape了
		pView->initBackground();
	}
}