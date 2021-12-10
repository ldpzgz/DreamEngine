#ifndef _BACKGROUND_H_
#define _BACKGROUND_H_
#include "../Texture.h"
#include "../Mesh.h"
#include "Shape.h"
#include <memory>

/*
* 表示view的背景，
* shape表示一个形状，可以是矩形，圆角矩形，椭圆，用一个MeshFilledRect绘制出来
* shape可以有一个边框，这个边框也是用一个MeshFilledRect绘制出来
* shape的位置宽高如何确定：
*	x = rect.x + paddingLeft;
*	y = rect.y + paddingTop;
*	width -= (paddingLeft + paddingRight);  //width是view的宽度，padding是shape的属性
*	height -= (paddingTop+ paddingBottom);  //height是view的高度
* 
* mpMesh有三种填充模式：纹理、渐变色、纯色
*	1 如果mpTexture不为空，则mpMesh使用posTexture material来渲染
*	2 如果shape设置了渐变属性，则mpMesh使用posColor material来渲染
*	3 如果shape设置了纯色属性，则mpMesh使用posUniformColor material来渲染
*/
class Background {
public:
	struct BackgroundStyle {
		std::shared_ptr<Texture> mpTex;
		Color mStartColor;
		Color mCenterColor;
		Color mEndColor;
		Color mSolidColor;
		Color mSrockeColor;
	};
	std::shared_ptr<Shape> mpShape;//常规的样子

	std::unique_ptr<BackgroundStyle> mpNormalStyle;
	std::unique_ptr<BackgroundStyle> mpPushedStyle;
	std::unique_ptr<BackgroundStyle> mpHoverStyle;
	std::unique_ptr<BackgroundStyle> mpDisabledStyle;
	static void getBkFromFile(const string& path,
		unordered_map<string, std::shared_ptr<Background>>& gBk);
	void nodeHandler(rapidxml::xml_node<char>* pAttribute);
	
};

#endif