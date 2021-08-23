#include "UiRender.h"
#include "../Log.h"
#include <fstream>
#include <vector>
#include <algorithm>
#include <cuchar>
#include "../Fbo.h"
#include "../Utils.h"
#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>  //rapidxml::file
#include <rapidxml_print.hpp>  //rapidxml::print
using namespace std;

const string CharSizeKey("charSize");		//一个字占用的宽高
static const string gUIRenderMaterialFile("./opengles3/material/uiDraw.material");
static const string gFontTextureName("fontsTexture");
static const string gFontFile("./opengles3/material/simfang.ttf");
static const string gSavedFontFile("./opengles3/material/myfont.data");
static const string gFontMaterialFile("./opengles3/material/font.material");
static const string gButtonMaterialFile("./opengles3/material/button.material");
static const string gResourceStringFile("./opengles3/material/strings.xml");
static const string gResourceColorFile("./opengles3/material/colors.xml");
FT_Library  FontInfo::glibrary;
FT_Face     FontInfo::gface;      /* handle to face object */
bool FontInfo::gIsFreetypeInit = false;



FontInfo::FontInfo(const shared_ptr<Texture>& pTex, const shared_ptr<Material>& pMaterial, const string& savePath_, const string& ttfPath, int charSize_) :
	pCharTexture(pTex),
	mpMaterial(pMaterial),
	savePath(savePath_),
	curTextureHeight(0),
	charSize(charSize_)
{
	initFreetype(ttfPath);
	if (pTex) {
		textureWidth = pTex->getWidth();
		textureHeight = pTex->getHeight();
	}
}

FontInfo::~FontInfo()
{
	releaseFreetype();
}

bool FontInfo::initFreetype(const string& ttfPath) {
	int error = 0;
	if (!gIsFreetypeInit)
	{

		error = FT_Init_FreeType(&glibrary);
		if (error)
		{
			LOGE("error to init freetype\n");
			return false;
		}

		error = FT_New_Face(glibrary,
			ttfPath.c_str(),
			0,
			&gface);
		if (error == FT_Err_Unknown_File_Format)
		{
			LOGE("error to FT_New_Face FT_Err_Unknown_File_Format \n");
			FT_Done_FreeType(glibrary);
			return false;
		}
		else if (error)
		{
			LOGE("error to FT_New_Face other \n");
			FT_Done_FreeType(glibrary);
			return false;
		}

		error = FT_Select_Charmap(gface, FT_ENCODING_UNICODE);
		if (error)
		{
			LOGE("error to FT_Select_Charmap \n");
			return false;
		}
	}
	gIsFreetypeInit = true;
	return true;
}

shared_ptr<FontInfo> FontInfo::loadFromFile(const string& savePath, const string& ttfPath, const string& materialPath) {

	auto pMaterial = make_shared<Material>();

	if (!pMaterial->parseMaterialFile(materialPath)) {
		LOGE("error to parse font material");
		return shared_ptr<FontInfo>();
	}

	auto pTex = Material::getTexture(gFontTextureName);
	if (!pTex) {
		LOGE("font texture %s not found in material %s",gFontTextureName.c_str(), materialPath.c_str());
		pMaterial.reset();
		return shared_ptr<FontInfo>();
	}

	auto charSize = pMaterial->getKeyAsInt(CharSizeKey);

	if (charSize < 0) {
		LOGE("not found charSize key in material %s", materialPath.c_str());
		pMaterial.reset();
		return shared_ptr<FontInfo>();
	}

	shared_ptr<FontInfo> fontInfo = make_shared<FontInfo>(pTex, pMaterial,savePath, ttfPath, charSize);

	fontInfo->mpCharMesh = make_shared<Mesh>(MeshType::MESH_FONTS);
	if (fontInfo->mpCharMesh) {
		fontInfo->mpCharMesh->loadMesh();
		fontInfo->mpCharMesh->setMaterial(pMaterial);
	}

	ifstream inFile(savePath, ios::in);
	if (inFile.is_open()) {
		//把字符信息取出来
		int countOfChar = 0;
		inFile >> countOfChar >> fontInfo->textureWidth >> fontInfo->textureHeight >> fontInfo->curTextureWidth >> fontInfo->curTextureHeight;
		if (fontInfo->textureWidth != fontInfo->pCharTexture->getWidth() ||
			fontInfo->textureHeight != fontInfo->pCharTexture->getHeight()) {
			LOGE("charTexture widht or height is not equal width the one defined in fonts Material file %s", materialPath.c_str());
			fontInfo.reset();
			return fontInfo;
		}
		for (int i = 0; i < countOfChar; ++i) {
			CharInTexture info;
			UnicodeType code;
			inFile.read((char*)&code, sizeof(UnicodeType));
			inFile >> info.x >> info.y >> info.left >> info.top >> info.width >> info.width >> info.advX >> info.advX;

			fontInfo->fontsMap.insert(make_pair(code, info));
		}
		
		//将文件中保存的渲染好的部分字符的图像读取出来
		int imageSize = fontInfo->textureWidth*fontInfo->curTextureHeight;
		std::vector<char> imageData;
		imageData.resize(imageSize);
		inFile.read(imageData.data(), imageSize);
		//更新纹理，把已经渲染好的字符图像上传到纹理上去
		fontInfo->pCharTexture->update(0, 0, fontInfo->textureHeight, fontInfo->curTextureHeight, imageData.data());
		inFile.close();
	}

	return fontInfo;
}

void FontInfo::getTextRenderInfo(const string& text, std::vector<CharRenderInfo>& renderInfoArray, const Rect<int>& rect){
	
}

void FontInfo::releaseFreetype()
{
	if (gIsFreetypeInit)
	{
		FT_Done_Face(gface);
		FT_Done_FreeType(glibrary);
	}
}

void FontInfo::saveToFile() {
	if (!fontsMap.empty()) {
		Fbo fbo;
		fbo.attachColorTexture(pCharTexture, 0);
		fbo.startRender();
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		std::vector<unsigned char> imageData;
		imageData.resize(textureWidth*curTextureHeight);
			glReadPixels(0, 0, textureWidth, curTextureHeight, GL_LUMINANCE, GL_UNSIGNED_BYTE, (void*)imageData.data());
		fbo.endRender();

		ofstream out(savePath, ios::out | ios::binary);
		size_t countOfChar = fontsMap.size();
		out << countOfChar << textureWidth << textureHeight << curTextureWidth << curTextureHeight;
		for_each(fontsMap.cbegin(), fontsMap.cend(), [&out](const FontsMapValueType& info) {
			out.write((const char*)&info.first, sizeof(UnicodeType));
			out<< info.second.x << info.second.y << info.second.left << info.second.top << info.second.width << info.second.width << info.second.advX << info.second.advX;
		});
		out.close();
	}
	
}

const CharInTexture& FontInfo::getCharInTexture(UnicodeType code) {
	auto it = fontsMap.find(code);
	if (it != fontsMap.end()) {
		return it->second;
	}
	else {
		//使用freetype渲染出来，然后更新到纹理上，记录字符在纹理上的位置信息。
		int error = 0;
		error = FT_Set_Pixel_Sizes(gface, charSize, charSize);
		if (error)
		{
			LOGE("error to FT_Set_Pixel_Sizes \n");
			throw error;
		}
		FT_Select_Charmap(gface, FT_ENCODING_UNICODE);
		error = FT_Load_Char(gface, code, FT_LOAD_RENDER);
		if (error)
		{
			LOGE("error to FT_Load_Char \n");
			throw error;
		}

		FT_GlyphSlot slot = gface->glyph;
		auto metrics = slot->metrics;
		CharInTexture info;
		/*CharInTexture info2;
		info2.advX = metrics.horiAdvance >> 6;
		info2.advY = 0;
		info2.left = metrics.horiBearingX >> 6;
		info2.top = metrics.horiBearingY >> 6;
		info2.width = metrics.width >> 6;
		info2.height = metrics.height >> 6;*/
		info.left = slot->bitmap_left;
		info.top = slot->bitmap_top;
		info.width = slot->bitmap.width;
		info.height = slot->bitmap.rows;
		info.advX = slot->advance.x >> 6;
		info.advY = slot->advance.y >> 6;
		if (curTextureHeight == 0) {
			curTextureHeight = charSize;
		}

		if (curTextureWidth + info.width > textureWidth) {//当前行写不下要换一行了
			curTextureWidth = 0;
			if (curTextureHeight + charSize > textureHeight)
			{
				//当前纹理已经写满了
				throw -1;
			}
			curTextureHeight += charSize;
		}

		info.x = curTextureWidth;
		info.y = curTextureHeight - charSize;
		
		//把字符点阵数据更新到纹理上。
		//由于opengl纹理坐标的原点是在左下角，上下颠倒一下图像
		std::vector<unsigned char> tempBuf;
		tempBuf.resize(info.width*info.height);
		unsigned char* pStart = slot->bitmap.buffer;
		unsigned char* pEnd = tempBuf.data();
		for (int i = info.height -1; i >=0; --i) {
			memcpy(pEnd, pStart + (slot->bitmap.pitch*i), info.width);
			pEnd += info.width;
		}
		pCharTexture->update(info.x, info.y, info.width, info.height, tempBuf.data());

		curTextureWidth += info.width;

		auto& ret = fontsMap.try_emplace(code, info);
		if (ret.second)
		{
			return ret.first->second;
			LOGD("success to insert map \n");
		}
		else {
			throw - 2;
		}
	}
}

unique_ptr<UiRender> UiRender::gInstance = make_unique<UiRender>();

void UiRender::initUiRender() {
	initTextView(gSavedFontFile, gFontFile, gFontMaterialFile);
	initButton(gButtonMaterialFile);
	mpLastMaterial = Material::loadFromFile(gUIRenderMaterialFile);
	mpLastMesh = make_shared<Mesh>(MeshType::MESH_FONTS);
	if (mpLastMesh) {
		mpLastMesh->loadMesh();
		mpLastMesh->setMaterial(mpLastMaterial);
	}
}

void UiRender::updateWidthHeight(float width, float height) {
	mWindowWidth = width;
	mWindowHeight = height;
	mProjMatrix = glm::ortho(0.0f, width, 0.0f, height,200.0f,-200.0f);
	mLastMeshModelMatrix = glm::identity<glm::mat4>();
	mLastMeshModelMatrix = glm::scale(mLastMeshModelMatrix, glm::vec3(width, height, 1.0f));
}

bool UiRender::initTextView(const string& savedPath, const string& ttfPath, const string& materialPath) {
	pFontInfo = FontInfo::loadFromFile(savedPath, ttfPath, materialPath);
	if (pFontInfo) {
		return true;
	}
	else {
		return false;
	}
}

bool UiRender::initButton(const string& buttonMaterial) {
	mpRectMesh = make_shared<Mesh>(MeshType::MESH_Rect);
	auto pMaterial = make_shared<Material>();
	if (!pMaterial->parseMaterialFile(buttonMaterial)) {
		LOGE("error to parse button material");
		return false;
	}
	else {
		mpRectMesh->setMaterial(pMaterial);
		return true;
	}
}

void UiRender::drawTextView(TextView* tv) {
	if (tv != nullptr && pFontInfo) {
		int originCharSize = pFontInfo->charSize;
		int textSize = tv->getTextSize();//字的点阵大小，像素为单位
		auto& tvRect = tv->getRect();
		const auto& text = tv->getText();
		auto slen = text.size();
		unsigned char* pStr = (unsigned char*)text.c_str();

		//float currentPosX = tvRect.x; //下一个要渲染的文字的位置x
		//float currentPosY = tvRect.y + tvRect.height / 2.0f + tvCharSize / 2.0f;//下一个要渲染的文字的位置x
		//currentPosY = mWindowHeight - currentPosY;//ui的坐标系跟OpenGL的坐标系不一样，需要转换一下
		//计算出要渲染的文字的纹理信息与位置信息，从(0,0)这个位置开始排版文字，等会再根据textview的属性，translate一下。
		float currentPosX = 0.0f; //下一个要渲染的文字的基线位置x
		float currentPosY = -textSize - tv->getLineSpacingInc();//下一个要渲染的文字的基线位置y
		float yAdvance = currentPosY;//考虑到textview设置的额外行间距，默认的行间距是freetype渲染文字时的CharSize
		float xExtraAdvance = tv->getCharSpacingInc();//考虑到textview设置的额外字符间距
		float maxWidth = tvRect.width;//textView的宽度，字符渲染不能超出这个宽度
		float maxHeight = tvRect.height;//textView的高度，字符渲染不能超出这个宽度
		float currentWidth = 0.0f;//当前已经渲染出去的字符的宽度，这个不能超出maxWidth
		int fontTextureWidth = pFontInfo->pCharTexture->getWidth();//保存字体位图的纹理的宽度
		int fontTextureHeight = pFontInfo->pCharTexture->getHeight();//保存字体位图的纹理的高度
		int tvMaxLines = tv->getMaxLines(); //textview设置的显示行数
		int currentLines = 1;			//
		int totalWidth = 0;				//字符串占用的宽度
		int totalHeight = currentPosY; //字符串占用的高度
		int maxYAdv = 0;//统计基线之下还有多少像素
		float scaleFactor = (float)textSize / (float)pFontInfo->charSize;
		std::vector<CharRenderInfo> charsRenderInfoArray;
		for(size_t i = 0; i<slen;){
			UnicodeType code;
			auto len = UtfConvert::utf(pStr, code);
			pStr += len;
			i += len;
			try {
				const auto& cinfo = pFontInfo->getCharInTexture(code);
				if (currentPosX + scaleFactor*cinfo.width>maxWidth) {//当前要渲染的文字会超出textview的边界
					if (currentLines >= tvMaxLines) {
						if (currentPosX >= maxWidth) {
							//渲染完毕
							break;
						}
					}
					else {
						//要换行了,增加一行
						currentPosX = 0;
						currentPosY += (yAdvance - maxYAdv);
						maxYAdv = 0;
						++currentLines;
						totalWidth = tvRect.width;
						totalHeight = currentPosY;
					}
				}

				auto maxYAdvTemp = scaleFactor*(cinfo.height - cinfo.top);
				if (maxYAdvTemp > maxYAdv) {
					maxYAdv = maxYAdvTemp;
				}
				//可以计算纹理矩阵了，mesh中的纹理坐标是(0,0)到(1,1)
				CharRenderInfo rInfo;
				
				rInfo.texMatrix = glm::translate(rInfo.texMatrix, glm::vec3((float)cinfo.x / (float)fontTextureWidth,
					(float)cinfo.y / (float)fontTextureHeight,0.0f));
				rInfo.texMatrix = glm::scale(rInfo.texMatrix, glm::vec3((float)cinfo.width / (float)fontTextureWidth,
					(float)cinfo.height / (float)fontTextureHeight,1.0f));

				//计算文字的model矩阵，渲染ui的时候使用的是正交投影，适配窗口的宽高
				//注意，ui坐标系统原点在左上角，y轴向下，与传统保持一直，openglY轴向上的
				rInfo.matrix = glm::translate(rInfo.matrix, glm::vec3(currentPosX+ scaleFactor*cinfo.left, currentPosY+ scaleFactor*(cinfo.top-cinfo.height), 0.0f));
				rInfo.matrix = glm::scale(rInfo.matrix, glm::vec3(scaleFactor*cinfo.width, scaleFactor*cinfo.height, 1.0f));

				currentPosX += (scaleFactor*cinfo.advX + xExtraAdvance);//考虑到textview设置的额外字符间距
				if (totalWidth < tvRect.width) {
					totalWidth = currentPosX;
				}
				charsRenderInfoArray.emplace_back(std::forward<CharRenderInfo>(rInfo));
			}
			catch (int error) {
				LOGE("getCharInfo error %d", error);
			}

		}
		//经过上面的处理，文字已经排版好了，但是整个文本的左上角在坐标原点(0,0)
		//需要根据textview的对齐属性（左对齐，右对齐，顶对齐，底对齐，水平居中，垂直居中）,结合textView的位置，确定平移向量，
		
		auto textGrivate = tv->getGravity();
		glm::vec3 moveVec(0.0f, 0.0f, 0.0f);
		if (textGrivate & LayoutParam::Center) {
			if (totalWidth > tvRect.width) {
				moveVec.x = tvRect.x;
			}
			else {
				moveVec.x = tvRect.x + (float)(tvRect.width- totalWidth)/2.0f;
			}

			if (totalHeight > tvRect.height) {
				moveVec.y = mWindowHeight - tvRect.y;
			}
			else {
				moveVec.y = mWindowHeight - tvRect.y - (float)(tvRect.height - totalHeight) / 2.0f;
			}
			
		}
		else if (textGrivate == LayoutParam::TopCenter) {
			//todo
		}
		else if (textGrivate == LayoutParam::LeftCenter) {
			//todo
		}

		//根据对齐属性，调整每一个的位置,并且渲染
		//glEnable(GL_SCISSOR_TEST);
		//glScissor(tvRect.x, mWindowHeight-tvRect.y, tvRect.width, tvRect.height);
		float trx = 0.0f;
		for (auto it = charsRenderInfoArray.begin(); it != charsRenderInfoArray.end(); ++it) {
			glm::mat4 moveToScreenMatrix(1.0f);
			moveToScreenMatrix = glm::translate(moveToScreenMatrix, moveVec);
			//it->matrix = glm::translate(it->matrix, moveVec);
			//绘制
			if (pFontInfo->mpMaterial) {
				pFontInfo->mpMaterial->updateUniformColor(tv->getTextColor());
			}
			if (pFontInfo->mpCharMesh) {
				pFontInfo->mpCharMesh->render(mProjMatrix*moveToScreenMatrix*it->matrix, it->texMatrix);
			}
		}
		//glDisable(GL_SCISSOR_TEST);
		//处理完毕
	}
}

void UiRender::drawButton(Button* bt){
	//先绘制背景，再绘制文字
	//先算出button的model矩阵
	auto& rect = bt->getRect();
	glm::mat4 model(1.0f);
	model = glm::scale(model, glm::vec3(rect.width, rect.height, 1.0f));
	model = glm::translate(model, glm::vec3(rect.x,mWindowHeight-rect.y,0.0f));
	if (mpRectMesh) {
		mpRectMesh->render(mProjMatrix*model);
		drawTextView(bt);
	}
}

void UiRender::drawLinearLayout(LinearLayout* pll) {
	auto& children = pll->getChildren();
	for (auto& pChild : children) {
		if (pChild) {
			pChild->draw();
		}
	}
}

void UiRender::drawUi() {
	if (mpLastMesh) {
		//todo关闭深度测试，等等
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glBlendEquation(GL_FUNC_ADD);
		mpLastMesh->render(mProjMatrix*mLastMeshModelMatrix);
		glEnable(GL_DEPTH_TEST);
		glDisable(GL_BLEND);
	}
}

unique_ptr<UiManager> UiManager::gInstance = make_unique<UiManager>();
unordered_map<string, string> UiManager::gRStrings;
unordered_map<string, Color> UiManager::gRColors;
void UiManager::parseRColors(const string& path) {
	rapidxml::file<> fdoc(path.c_str());
	if (fdoc.size() > 0) {
		rapidxml::xml_document<> doc;// character type defaults to char
		doc.parse<0>(fdoc.data());// 0 means default parse flags
		auto pResNode = doc.first_node("resources");
		if (pResNode != nullptr) {
			auto pColorNode = pResNode->first_node("color");
			while (pColorNode != nullptr) {
				auto attribute = pColorNode->first_attribute("name");
				if (attribute != nullptr) {
					string key = attribute->value();
					string value = pColorNode->value();//value is like #ffffffaa
					if (!key.empty() && !value.empty()) {
						Color color;
						if (Color::parseColor(value, color)) {
							auto ret = gRColors.try_emplace(key, color);
							if (!ret.second) {
								LOGD("there already has color who's name is %s in gRColors", key.c_str());
							}
						}
						else {
							LOGE("parse color %s in colors.xml", key.c_str());
						}
					}
				}
				pColorNode = pColorNode->next_sibling();
			}
		}
	}
	else {
		LOGE("error to parse resource string file %s",path);
	}
}
void UiManager::parseRStrings(const string& path) {
	shared_ptr<rapidxml::file<>> pfdoc;
	try {
		pfdoc = make_shared<rapidxml::file<>>(path.c_str());
	}
	catch (std::exception e) {
		LOGE("error to parseRStrings %s file,error %s", path.c_str(),e.what());
		return;
	}
	
	if (pfdoc && pfdoc->size() > 0) {
		rapidxml::xml_document<> doc;// character type defaults to char
		doc.parse<0>(pfdoc->data());// 0 means default parse flags
		auto pResNode = doc.first_node("resources");
		if (pResNode != nullptr) {
			auto pStringNode = pResNode->first_node("string");
			while (pStringNode != nullptr) {
				auto attribute = pStringNode->first_attribute("name");
				if (attribute != nullptr) {
					string key = attribute->value();
					string value = pStringNode->value();//value is like #ffffffaa
					if (!key.empty() && !value.empty()) {
						auto ret = gRStrings.try_emplace(key, value);
						if (!ret.second) {
							LOGE("error to store string resource %s",key.c_str());
						}
					}
				}
				pStringNode = pStringNode->next_sibling();
			}
		}
	}
}

void parseView(const shared_ptr<View>& parent, rapidxml::xml_node<char>* pnode, shared_ptr<UiTree>& mpTree) {
	shared_ptr<View> pView;
	if (pnode != nullptr) {
		string viewName = pnode->name();
		pView = View::createView(viewName, nullptr);
		
		if (pView) {
			pView->setDirtyListener(mpTree);
			if (parent) {
				pView->setParent(parent);
				parent->addChild(pView);
			}
			else{
				mpTree->mpRootView = pView;
			}

			auto attr = pnode->first_attribute();
			while (attr != nullptr) {
				string attrName = attr->name();
				if (attrName == "id") {
					//有id的控件，才保存起来，以便查找
					mpTree->mViews.emplace(attr->value(), pView);
				}
				else {
					auto it = View::gAttributeHandler.find(attrName);
					if (it != View::gAttributeHandler.end()) {
						it->second(pView, attr->value());
					}
					else {
						LOGD("there are no %s attributeHandler,please supplement", attrName.c_str());
					}
				}
				attr = attr->next_attribute();
			}
		}
		auto child = pnode->first_node();
		auto sibling = pnode->next_sibling();
		if (child != nullptr) {
			parseView(pView, child, mpTree);
		}
		if (sibling != nullptr) {
			parseView(parent, sibling, mpTree);
		}
	}
}

//加载ui的布局文件
shared_ptr<UiTree> UiManager::loadFromFile(const string& filepath) {
	shared_ptr<UiTree> mpTree = make_shared<UiTree>();
	//读取xml
	rapidxml::file<> fdoc(filepath.c_str());
	if (fdoc.size() > 0) {
		rapidxml::xml_document<> doc;// character type defaults to char
		doc.parse<0>(fdoc.data());// 0 means default parse flags

		auto root = doc.first_node();
		parseView(shared_ptr<View>(), root, mpTree);
	}
	return mpTree;
}

UiManager::UiManager() {

}

UiManager::~UiManager() {

}

void UiManager::setUiTree(const shared_ptr<UiTree>& tree) {
	mpUiTree = tree;
	if (mpUiTree) {
		//计算出view的位置尺寸
		mpUiTree->updateWidthHeight(mWindowWidth, mWindowHeight);
		mpUiTree->calcViewsRect(mWindowWidth, mWindowHeight);
		/*if (mpUiTree->mpRootView) {
			mpUiTree->mpRootView->setDirty(true);
		}*/
		UiRender::getInstance()->setTexture(mpUiTree->mpTexture);
	}
}

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

void UiManager::updateWidthHeight(float width, float height) {
	mWindowWidth = width;
	mWindowHeight = height;
	UiRender::getInstance()->updateWidthHeight(width, height);
	if (mpUiTree) {
		mpUiTree->updateWidthHeight(width, height);
	}
}

void UiManager::mouseMove(int x, int y) {
	if (mpUiTree) {
		if (mpUiTree->mpRootView) {
			mpUiTree->mpRootView->mouseMove( x, y);
			auto children = mpUiTree->mpRootView->getChildren();
			for (auto& child : children) {
				if (child) {
					child->mouseMove( x, y);
				}
			}
		}
	}
}

void UiManager::mouseLButtonDown(int x, int y) {
	if (mpUiTree) {
		if (mpUiTree->mpRootView) {
			mpUiTree->mpRootView->mouseLButtonDown( x, y);
			auto children = mpUiTree->mpRootView->getChildren();
			for (auto& child : children) {
				if (child) {
					child->mouseLButtonDown( x, y);
				}
			}
		}
	}
}

void UiManager::mouseLButtonUp(int x, int y) {
	if (mpUiTree) {
		if (mpUiTree->mpRootView) {
			mpUiTree->mpRootView->mouseLButtonUp( x, y);
			auto children = mpUiTree->mpRootView->getChildren();
			for (auto& child : children) {
				if (child) {
					child->mouseLButtonUp( x, y);
				}
			}
		}
	}
}

bool UiManager::initUi(int w, int h) {
	//初始化uirender
	UiRender::getInstance()->initUiRender();
	//加载ui string和color配置
	parseRStrings(gResourceStringFile);
	parseRColors(gResourceColorFile);
	updateWidthHeight(w, h);
	return true;
}

void UiManager::draw() {
	if (mpUiTree) {
		//这个是将uitree绘制到纹理
		mpUiTree->draw();
		//这个是把纹理显示出来
		UiRender::getInstance()->drawUi();
	}
}