#ifndef _UI_TREE_H_
#define _UI_TREE_H_
#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <string>
#include <unordered_map>
#include <memory>
#include "../material.h"
#include "../Fbo.h"
#include "../Texture.h"
#include "View.h"

using namespace std;

class UiTree : public DirtyListener,public std::enable_shared_from_this< DirtyListener >{
public:
	//应该把uiTree绘制到一张纹理上，避免每次都去绘制所有ui，很多时候，ui是没有变化的，
	//只更新有变化的ui
	bool draw();
	void addDirtyView(const shared_ptr<View>& pView) override;
	/*
	* 创建fbo，支持渲染到多重采样纹理
	*/
	void updateWidthHeight(float width, float height);

	//设置rootView，并为每一个view设置dirty监听器
	void setRootView(const std::shared_ptr<View>& pRootView);

	/*
	计算uitree上所有view的实际尺寸
	*/
	void calcViewsRect(int windowWidth, int windowHeight);
//private:
//	void calcViewsWidthHeight(int parentWidth, int parentHeight, shared_ptr<View> pView);
//	void calcViewsPos(shared_ptr<View> pView);
public:
	shared_ptr<View> mpRootView;
	list<weak_ptr<View>> mViewsToBeDrawing;
	Fbo mFboForRender;
	shared_ptr<Texture> mpTexture;
	bool mbRedraw{ false };
};
#endif