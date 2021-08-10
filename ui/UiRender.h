#ifndef _UI_RENDER_H_
#define _UI_RENDER_H_
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <locale.h>
#include <iostream>
#include <string>
#include <map>
#include <memory>
#include "../material.h"
#ifndef GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
// Include all GLM extensions
#include <glm/ext.hpp> // perspective, translate, rotate
#include <glm/gtx/matrix_transform_2d.hpp>
#endif
#include "../Utils.h"
#include "../Mesh.h"
#include "../Node.h"
using namespace std;
using UnicodeType = char32_t;
//void initUi();

//一个字符的位图信息，描述了该字符在纹理里面的位置，宽高等信息
class CharInTexture
{
public:
	int x;
	int y;		//(x,y)表示字符在纹理中的起始位置
	int width;	//字符的宽度
	int height;	//字符的高度,x,y,width,height取值范围都是[0,1]
	int left;		//left
	int top;		//top,相对于基线的高度，单位是像素，用于实际渲染
	
	int advX; //表示下一个字符与本字符的水平距离，单位是像素
	int advY; //表示下一个字符与本字符的垂直距离，单位是像素
};

//一个字符的渲染信息
class CharRenderInfo {
public:
	glm::mat4 matrix;//描述了字符的位置，缩放，旋转等信息，这个矩阵负责把一个0，1的矩形绘制到目的地。
	glm::mat3 texMatrix;//这个矩阵负责把一个0，1的纹理坐标，变换到该字符所占用的纹理区域
};

/*
这个类不要自己单独创建，UiRender::initUiRender函数里面会创建它。
*/
class FontInfo
{
public:
	FontInfo(const shared_ptr<Texture>& pTex, const shared_ptr<Material>& pMaterial,const string& filePath, const string& ttfPath,int charSize_);
	~FontInfo();
	/*
	功能：从filePath加载预渲染好的文字信息，后面渲染好的文字，会保存到这个文件里面
	filePath	自定义的用于保存文字信息的文件的路径
	ttfPath		用于渲染文字的ttf等字体文件的路径
	materialPath渲染字体用的材质文件，里面会定义保存字符的纹理，每个字符的大小等信息
	*/
	static shared_ptr<FontInfo> loadFromFile(const string& savedPath, const string& ttfPath,const string& materialPath);

	/*
	text 为utf-8编码的字符
	*/
	void getTextRenderInfo(const string& text,std::vector<CharRenderInfo>& renderInfoArray,const Rect<int>& rect);

	const CharInTexture& getCharInTexture(UnicodeType code);

	void saveToFile();

	int textureWidth{ 0 };
	int textureHeight{ 0 };
	int curTextureHeight{ 0 }; //pCharTexture纹理里面当前已经被占用的高度
	int curTextureWidth{ 0 };  //pCharTexture纹理里面当前行的宽度
	int charSize{32};
	string savePath;
	shared_ptr<Mesh> mpCharMesh;//用于承载当个字符的mesh
	shared_ptr<Material> mpMaterial;
	shared_ptr<Texture> pCharTexture;
	map<UnicodeType, CharInTexture> fontsMap;

	using FontsMapValueType = map<UnicodeType, CharInTexture>::value_type;

	static 	FT_Library  glibrary;
	static  FT_Face     gface;      /* handle to face object */
	static  bool gIsFreetypeInit;
private:
	static bool initFreetype(const string& ttfPath);
	static void releaseFreetype();
};

class TextView;
class Button;
class LinearLayout;
/*
规定UI的坐标系，原点在左上角，Y轴向下。
*/
class UiRender {
public:
	static unique_ptr<UiRender>& getInstance() {
		return gInstance;
	}

	/*
	功能：		初始化textview
	savedPath	保存已经渲染好的字体信息的文件
	ttfPath		使用那个字体文件来渲染字体
	materialPath 渲染文字使用的material
	*/
	bool initTextView(const string& savedPath, const string& ttfPath, const string& materialPath);
	/*
	功能：			初始化button
	buttonMaterial	渲染button使用的material
	*/
	bool initButton(const string& buttonMaterial);

	void updateWindowSize(float w, float h) {
		mWindowWidth = w;
		mWindowHeight = h;
	}

	void updateProjMatrix(const glm::mat4& mat) {
		mProjMatrix = mat;
	}

	void drawTextView(TextView* tv);
	void drawButton(Button* tv);
	void drawLinearLayout(LinearLayout* pll);
private:
	static unique_ptr<UiRender> gInstance;

	shared_ptr<FontInfo> pFontInfo;
	shared_ptr<Mesh> mpRectMesh;//用于承载Button的背景

	float mWindowWidth;
	float mWindowHeight;
	glm::mat4 mProjMatrix;
};

/*
这个类有一颗ui树，ui树上每个node可以挂一个view，
遍历这棵树，调用每个view的render函数，绘制出ui。
负责传递输入事件给每一个view
*/
class UiManager {
public:
	//从一个xml文件里面加载一棵ui树，准备模仿Android的ui系统
	static shared_ptr<Node<glm::mat3>> loadFromFile(const string& filepath);

	void setUiRootNode(const shared_ptr<Node<glm::mat3>> pRoot) {
		mpRootNode = pRoot;
	}

	UiManager();

	~UiManager();

	void rendUI();
private:
	void renderNode(shared_ptr<Node<glm::mat3>>& pNode);

	std::shared_ptr<Node<glm::mat3>> mpRootNode;
	float mWindowWidth;
	float mWindowHeight;
	glm::mat4 mProjMatrix;
};

#endif