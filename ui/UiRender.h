#ifndef _UI_RENDER_H_
#define _UI_RENDER_H_
#include "FreetypeUtils.h"
#include "CharPosition.h"
#include "CharInfo.h"
#include "../Utils.h"
#include "../Mesh.h"
#include "../Node.h"
#include "../Fbo.h"
#include "../Texture.h"
#include "View.h"
#include "TextView.h"
#include "Button.h"
#include "LinearLayout.h"
#include "Shape.h"
#include "TextGpuRender.h"

using namespace std;

/*
这个类不要自己单独创建，UiRender::initUiRender函数里面会创建它。
*/
class FontManager
{
public:
	FontManager();
	~FontManager();
	/*
	功能：从filePath加载预渲染好的文字信息，后面渲染好的文字，会保存到这个文件里面
	filePath	自定义的用于保存文字信息的文件的路径
	ttfPath		用于渲染文字的ttf等字体文件的路径
	materialPath渲染字体用的材质文件，里面会定义保存字符的纹理，每个字符的大小等信息
	*/
	static shared_ptr<FontManager> loadFromFile(const string& savedPath, const string& ttfPath,const string& materialName);

	bool initFontManager(const shared_ptr<Texture>& pTex, const shared_ptr<Material>& pMaterial, const string& filePath, const string& ttfPath, int charSize_);

	bool getCharInTexture(UnicodeType code,CharInfo& info);

	//const CharInfo& getCharIntexture2(UnicodeType code);

	void saveToFile();

	int mTextureWidth{ 0 };
	int mTextureHeight{ 0 };
	int mCurTextureHeight{ 0 }; //pCharTexture纹理里面当前已经被占用的高度
	int mCurTextureWidth{ 0 };  //pCharTexture纹理里面当前行的宽度
	int mCharSize{32};
	string mSavePath;
	shared_ptr<Mesh> mpCharMesh;//用于承载当个字符的mesh,用于渲染textview里面的文字
	shared_ptr<Material> mpMaterial;
	shared_ptr<Texture> mpCharTexture;
	map<UnicodeType, CharInfo> mFontsMap;
	TextGpuRenderP mpTextGpuRender;
};

//class View;
//class TextView;
//class Button;
//class LinearLayout;
/*
规定UI的坐标系，原点在左上角，Y轴向下。
*/
class UiRender {
public:
	UiRender() :
		mLastMeshModelMatrix(1.0f),
		mWindowWidth(0.0f),
		mWindowHeight(0.0f),
		mProjMatrix(1.0f)
	{

	}
	static unique_ptr<UiRender>& getInstance() {
		return gInstance;
	}

	void initUiRender();

	/*
	功能：		初始化textview
	savedPath	保存已经渲染好的字体信息的文件
	ttfPath		使用那个字体文件来渲染字体
	materialPath 渲染文字使用的material
	*/
	bool initTextView(const string& savedPath, const string& ttfPath, const string& materialName);

	//确定了shape的宽高和中心点之后才能调用这个函数
	void initBackground(View* pView);

	//当窗口变化的时候，需要调用这个函数更新一下
	void updateWidthHeight(float width, float height);

	//宽度或者高度为wrapContent的时候，计算TextView文本的宽度和高度以像素为单位
	void calcTextViewWidthHeight(TextView* tv);
	void drawTextView(TextView* tv);
	void calcTextPosition(TextView* tv);
	void drawBackground(View* tv);
	void drawLinearLayout(LinearLayout* pll);

	//指定最后要显示出来的纹理，当前uitree的纹理，每棵ui树都会渲染到它自己的纹理上面。
	void setTexture(const shared_ptr<Texture>& pTex) {
		if (mpLastMaterial) {
			mpLastMaterial->setTextureForSampler("s_texture", pTex);
		}
	}

	//完成最后的ui绘制工作，先前已经将uitree绘制到纹理里面了
	void drawUi();
private:
	static unique_ptr<UiRender> gInstance;

	shared_ptr<FontManager> mpFontManager;
	shared_ptr<Material> mpLastMaterial;//渲染最终的ui的材质
	shared_ptr<Mesh> mpLastMesh;//渲染最终的ui的材质mesh
	glm::mat4 mLastMeshModelMatrix;
	float mWindowWidth;
	float mWindowHeight;
	glm::mat4 mProjMatrix;
};
#endif