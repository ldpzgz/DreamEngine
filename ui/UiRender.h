#ifndef _UI_RENDER_H_
#define _UI_RENDER_H_
#include <ft2build.h>
#include FT_FREETYPE_H
#include <stdio.h>
#include <locale.h>
#include <iostream>
#include <string>
#include <map>
#include <unordered_map>
#include <memory>
#include "../material.h"
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
// Include all GLM extensions
#include <glm/ext.hpp> // perspective, translate, rotate
#include <glm/gtx/matrix_transform_2d.hpp>

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

class View;
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
	功能：			初始化绘制button需要用到的一些资源
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

class UiTree {
public:
	shared_ptr<View> mpRootView;
	unordered_map<std::string, shared_ptr<View>> mViews;//存储拥有id的view
};

/*
这个类有一颗ui树，ui树上每个node可以挂一堆view，
遍历这棵树，调用每个view的render函数，绘制出ui。
负责传递输入事件给每一个view
*/
class UiManager {
public:
	static unique_ptr<UiManager>& getInstance() {
		return gInstance;
	}

	//从一个xml文件里面加载一棵ui树，准备模仿Android的ui系统
	shared_ptr<UiTree> loadFromFile(const string& filepath);

	/*
	功能：	初始化ui，初始化uirender，加载string.xml,color.xml等
	w		窗口的宽度
	h		窗口的高度
	*/
	bool initUi(int w,int h);

	UiManager();

	~UiManager();

	//设置ui树，计算ui树中每个控件的位置尺寸，设置进来的根节点会被渲染
	void setUiTree(const shared_ptr<UiTree>& tree);

	//当窗口变化的时候，需要调用这个函数更新一下
	void updateWidthHeight(float width, float height);

	//应该把ui绘制到一张纹理上，避免每次都去绘制所有ui，很多时候，ui是没有变化的，
	//只更新有变化的ui
	void rendUI();

	//ui的输入事件驱动
	void mouseMove(int x, int y);

	void mouseLButtonDown(int x, int y);

	void mouseLButtonUp(int x, int y);
private:
	/*
	计算uitree上所有view的实际尺寸
	*/
	void calcViewsRect();
	void calcViewsWidthHeight(int parentWidth, int parentHeight, shared_ptr<View> pView);
	void calcViewsPos(shared_ptr<View> pView);
	shared_ptr<UiTree> mpUiTree;
	float mWindowWidth;
	float mWindowHeight;
	glm::mat4 mProjMatrix;

	static unique_ptr<UiManager> gInstance;
	static unordered_map<string, string> gRStrings;//保存从material/strings.xml里面解析出来的字符串
	static unordered_map<string, Color> gRColors;//保存从material/colors.xml里面解析出来的颜色值
	static void parseRStrings(const string& path);
	static void parseRColors(const string& path);
};

#endif