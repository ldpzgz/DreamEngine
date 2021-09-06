#ifndef _UI_MANAGER_H_
#define _UI_MANAGER_H_
#include "../Utils.h"
#include "../Mesh.h"
#include "../Node.h"
#include "../Fbo.h"
#include "../Texture.h"
#include "UiTree.h"
#include "UiRender.h"
#include "Shape.h"

using namespace std;
using UnicodeType = char32_t;

/*
这个类有一颗ui树，ui树上每个node可以挂一堆view，
遍历这棵树，调用每个view的render函数，绘制出ui。
负责传递输入事件给每一个view
用法：
	//加载布局文件，初始化
	UiManager::getInstance()->initUi(w,h);
	auto ptree = UiManager::loadFromFile(layoutfile);
	UiManager::getInstance()->setUiTree(ptree);
	

	//窗口宽高变化的时候调用
	UiManager::getInstance()->updateWidthHeight(w,h);

	//鼠标事件
	UiManager::getInstance()->mouseMove(x,y);
	UiManager::getInstance()->mouseLButtonDown(x,y);
	UiManager::getInstance()->mouseLButtonUp(x,y);

	//绘制ui
	UiManager::getInstance()->draw();
*/
class UiManager {
public:
	static unique_ptr<UiManager>& getInstance() {
		return gInstance;
	}

	//从一个xml文件里面加载一棵ui树，准备模仿Android的ui系统
	static shared_ptr<UiTree> loadFromFile(const string& filepath);

	/*
	功能：	初始化ui，初始化uirender，加载string.xml,color.xml等
	w		窗口的宽度
	h		窗口的高度
	*/
	bool initUi(int w,int h);
	//先将mpUiTree绘制到纹理里，只绘制需要更新的ui
	//再输出到屏幕
	void draw();

	UiManager();

	~UiManager();

	//设置ui树，计算ui树中每个控件的位置尺寸，设置进来的根节点会被渲染
	void setUiTree(const shared_ptr<UiTree>& tree);

	//当窗口变化的时候，需要调用这个函数更新一下
	void updateWidthHeight(float width, float height);

	//ui的输入事件驱动
	void mouseMove(int x, int y);

	void mouseLButtonDown(int x, int y);

	void mouseLButtonUp(int x, int y);

	static Color& getColor(const std::string& name);
	static std::shared_ptr<Shape>& getShape(const std::string& name);
	static std::string& getString(const std::string& name);
	static std::shared_ptr<Texture>& getTexture(const std::string& name);
private:
	shared_ptr<UiTree> mpUiTree;
	//Fbo mFboForCopy;
	/*
	用于接收mpUiTree（渲染到rbo里面的）的渲染结果
	*/
	//shared_ptr<Texture> mpTexture;
	float mWindowWidth;
	float mWindowHeight;
	glm::mat4 mProjMatrix;

	static unique_ptr<UiManager> gInstance;
	static unordered_map<string, string> gRStrings;//保存从material/strings.xml里面解析出来的字符串
	static unordered_map<string, Color> gRColors;//保存从material/colors.xml里面解析出来的颜色值
	static unordered_map<string, std::shared_ptr<Shape>> gRShapes;
	static void parseRStrings(const string& path);
	static void parseRColors(const string& path);
	static void parseRShape(const string& path);
	static void loadAllUiImage();
	static void loadAllShape();
};

#endif