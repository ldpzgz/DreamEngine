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

class UiTree : public DirtyListener {
public:
	//应该把uiTree绘制到一张纹理上，避免每次都去绘制所有ui，很多时候，ui是没有变化的，
	//只更新有变化的ui
	bool draw();
	void addDirtyView(const shared_ptr<View>& pView) override;
	void updateWidthHeight(float width, float height);

	/*
	计算uitree上所有view的实际尺寸
	*/
	void calcViewsRect(int windowWidth, int windowHeight);
	void calcViewsWidthHeight(int parentWidth, int parentHeight, shared_ptr<View> pView);
	void calcViewsPos(shared_ptr<View> pView);

	shared_ptr<View>& findViewById(const std::string& id);

	shared_ptr<View> mpRootView;
	unordered_map<std::string, shared_ptr<View>> mViews;//存储拥有id的view
	list<weak_ptr<View>> mViewsToBeDrawing;
	Fbo mFboForRender;
	shared_ptr<Texture> mpTexture;
	bool mbRedraw{ false };
};
#endif