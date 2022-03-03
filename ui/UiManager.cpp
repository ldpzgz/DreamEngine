#include "UiManager.h"
#include "Background.h"
#include "../Log.h"
#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>  //rapidxml::file
#include <rapidxml_print.hpp>  //rapidxml::print
#include <filesystem>
#include "../Utils.h"
using namespace std::filesystem;
using namespace std;

static const string gStringFile("./opengles3/material/strings.xml");
static const string gColorFile("./opengles3/material/colors.xml");
static const string gShapePath("./opengles3/material/shape");
static const string gBackgroundPath("./opengles3/material/background");
static const string gUiImagePath("./opengles3/material/drawable/uiImage");
static const string gUiLayoutPath("./opengles3/material/layout");

unique_ptr<UiManager> UiManager::gInstance = make_unique<UiManager>();
unordered_map<string, string> UiManager::gRStrings;
unordered_map<string, Color> UiManager::gRColors;
unordered_map<string, std::shared_ptr<Shape>> UiManager::gRShapes;
unordered_map<string, std::shared_ptr<Background>> UiManager::gRBackground;

void UiManager::loadAllBackground() {
	path bkPath(gBackgroundPath);
	if (!exists(bkPath)) {
		LOGE("ERROR the ui shape path %s is not exist", gShapePath.c_str());
	}

	if (is_directory(bkPath)) {
		//是目录
		directory_iterator list(bkPath);
		//directory_entry 是一个文件夹里的某一项，可以是path，也可以是文件
		for (auto& it : list) {
			auto& filePath = it.path();
			if (is_regular_file(filePath)) {
				//是文件
				auto filePathString = filePath.string();
				parseRBackground(filePathString);
			}
		}
	}
}

void UiManager::loadAllShape() {
	//遍历UIimage目录
	path shapePath(gShapePath);
	if (!exists(shapePath)) {
		LOGE("ERROR the ui shape path %s is not exist", gShapePath.c_str());
	}

	if (is_directory(shapePath)) {
		//是目录
		directory_iterator list(shapePath);
		//directory_entry 是一个文件夹里的某一项，可以是path，也可以是文件
		for (auto& it : list) {
			auto& filePath = it.path();
			if (is_regular_file(filePath)) {
				//是文件
				auto filePathString = filePath.string();
				parseRShape(filePathString);
			}
		}
	}
}

void UiManager::loadAllUiImage() {
	//遍历UIimage目录
	path imagePath(gUiImagePath);
	if (!exists(imagePath)) {
		LOGE("ERROR the ui image path %s is not exist", gUiImagePath.c_str());
	}

	if (is_directory(imagePath) ){
		//是目录
		directory_iterator list(imagePath);
		//directory_entry 是一个文件夹里的某一项，可以是path，也可以是文件
		for (auto& it : list) {
			auto filePath = it.path();
			if (is_regular_file(filePath)) {
				//是文件
				auto filePathString = filePath.string();
				auto texName = "ui"+Utils::getFileName(filePathString);
				Material::loadImageFromFile(filePathString,texName);
			}
		}
	}
}

void UiManager::parseRShape(const string& path) {
	unique_ptr<rapidxml::file<>> pfdoc;
	try {
		pfdoc = make_unique<rapidxml::file<>>(path.c_str());
	}
	catch (std::exception e) {
		LOGE("error to parseRShape %s file,error %s", path.c_str(), e.what());
		return;
	}
	if (pfdoc && pfdoc->size() > 0) {
		string shapeName = Utils::getFileName(path);
		auto pDoc = make_unique< rapidxml::xml_document<> >();// character type defaults to char
		pDoc->parse<0>(pfdoc->data());// 0 means default parse flags
		auto pResNode = pDoc->first_node("shape");
		if (pResNode != nullptr) {
			auto shape = std::make_shared<Shape>();
			auto attribute = pResNode->first_attribute();
			while (attribute != nullptr) {
				string key = attribute->name();
				string value = attribute->value();
				auto it = Shape::gShapeAttributeHandler.find(key);
				if (it != Shape::gShapeAttributeHandler.end()) {
					it->second(shape, value);
				}
				else {
					LOGE("cannot recognize shape attribute %s", key.c_str());
				}
				attribute = attribute->next_attribute();
			}
			
			if (gRShapes.emplace(shapeName, shape).second == false) {
				LOGE("ERROR to emplace shape %s ,already exist", shapeName.c_str());
				shape.reset();
			}
		}
	}
}
//处理background xml文件里面的属性
void shapeHandler(std::unique_ptr<Background::BackgroundStyle>& pStyle, const string& value) {
	if (pStyle) {
		auto& pShape = UiManager::getShape(value);
		if (pShape) {
			//shape 属性必须放在background属性里面的第一个位置
			pStyle->setShape(pShape);
			pStyle->setTexture(pShape->getTexture());
			pStyle->setSolidColor(pShape->getSolidColor());
			pStyle->setStartColor(pShape->getGradientStartColor());
			pStyle->setCenterColor(pShape->getGradientCenterColor());
			pStyle->setEndColor(pShape->getGradientEndColor());
			pStyle->setBorderColor(pShape->getBorderColor());
		}
		else {
			LOGE("error when parse background xml,shape %s not found",value.c_str());
		}
		
	}
}

void textureHandler(std::unique_ptr<Background::BackgroundStyle>& pStyle, const string& value) {
	if (pStyle) {
		pStyle->setTexture(UiManager::getTexture(value));
	}
}

void startColorHandler(std::unique_ptr<Background::BackgroundStyle>& pStyle, const string& value) {
	if (pStyle) {
		Color::parseColor(value, pStyle->mStartColor);
	}
}

void centerColorHandler(std::unique_ptr<Background::BackgroundStyle>& pStyle, const string& value) {
	if (pStyle) {
		Color::parseColor(value, pStyle->mCenterColor);
	}
}

void endColorHandler(std::unique_ptr<Background::BackgroundStyle>& pStyle, const string& value) {
	if (pStyle) {
		Color::parseColor(value, pStyle->mEndColor);
	}
}

void solidColorHandler(std::unique_ptr<Background::BackgroundStyle>& pStyle, const string& value) {
	if (pStyle) {
		Color::parseColor(value, pStyle->mSolidColor);
	}
}

void borderColorHandler(std::unique_ptr<Background::BackgroundStyle>& pStyle, const string& value) {
	if (pStyle) {
		Color::parseColor( value, pStyle->mBorderColor);
	}
}

unordered_map<string, std::function<void(std::unique_ptr<Background::BackgroundStyle>&, const string&)>> gStyleHandlers{
	{"shape",shapeHandler},
	{"texture",textureHandler},
	{"solidColor",solidColorHandler},
	{"startColor",startColorHandler},
	{"centerColor",centerColorHandler},
	{"endColor",endColorHandler},
	{"borderColor",borderColorHandler},
};

void backgroundAttributeHandler(std::unique_ptr<Background::BackgroundStyle>& pStyle, rapidxml::xml_attribute<char>* pAttribute) {
	auto endIt = gStyleHandlers.end();
	while (pAttribute) {
		string name = pAttribute->name();
		string value = pAttribute->value();
		auto it = gStyleHandlers.find(name);
		if (it != endIt) {
			it->second(pStyle, value);
		}
		else {
			LOGE("ERROR to parse background xml,the attribute %s is illeague", name.c_str());
			return;
		}
		pAttribute = pAttribute->next_attribute();
	}
}
void backgroundNodeHandler(shared_ptr<Background>& pBk,rapidxml::xml_node<char>* pNode) {
	if (pNode) {
		std::string name = pNode->name();
		if (name == "normal") {
			auto& pStyle = pBk->getNormalStyle();
			backgroundAttributeHandler(pStyle, pNode->first_attribute());
		}
		else if (name == "pushed") {
			auto& pStyle = pBk->getPushedStyle();
			pStyle = std::make_unique<Background::BackgroundStyle>();
			backgroundAttributeHandler(pStyle, pNode->first_attribute());
		}
		else if (name == "hover") {
			auto& pStyle = pBk->getHoverStyle();
			pStyle = std::make_unique<Background::BackgroundStyle>();
			backgroundAttributeHandler(pStyle, pNode->first_attribute());
		}
		else if (name == "disabled") {
			auto& pStyle = pBk->getDisabledStyle();
			pStyle = std::make_unique<Background::BackgroundStyle>();
			backgroundAttributeHandler(pStyle, pNode->first_attribute());
		}
	}
}
/*
* 分析bk配置文件，根据配置，给bk里面的各种style的变量赋上相应的值
*/
void UiManager::parseRBackground(const string& path) {
	shared_ptr<Background> pBack;
	unique_ptr<rapidxml::file<>> pfdoc;
	try {
		pfdoc = make_unique<rapidxml::file<>>(path.c_str());
	}
	catch (std::exception e) {
		LOGE("error to parseRBackground %s file,error %s", path.c_str(), e.what());
		return ;
	}
	if (pfdoc && pfdoc->size() > 0) {
		string bkName = Utils::getFileName(path);
		auto pDoc = make_unique< rapidxml::xml_document<> >();// character type defaults to char
		pDoc->parse<0>(pfdoc->data());// 0 means default parse flags
		auto pNode = pDoc->first_node("background");
		if (pNode != nullptr) {
			pBack = make_shared<Background>();
			pNode = pNode->first_node();
			while (pNode) {
				backgroundNodeHandler(pBack,pNode);
				pNode = pNode->next_sibling();
			}
			//有可能和normalstyle 共享shape
			auto& pNormalStyle = pBack->getNormalStyle();
			if (pNormalStyle) {
				auto& pShape = pNormalStyle->getShape();
				if (pShape) {
					auto& pPushedStyle = pBack->getPushedStyle();
					if (pPushedStyle) {
						if (!pPushedStyle->getShape()) {
							pPushedStyle->setShape(pShape);
						}
					}
					auto& pDisabledStyle = pBack->getDisabledStyle();
					if (pDisabledStyle) {
						if (!pDisabledStyle->getShape()) {
							pDisabledStyle->setShape(pShape);
						}
					}
					auto& pHoverStyle = pBack->getHoverStyle();
					if (pHoverStyle) {
						if (!pHoverStyle->getShape()) {
							pHoverStyle->setShape(pShape);
						}
					}
				}
			}
			gRBackground.emplace(bkName, pBack);
		}
	}
}

void UiManager::parseRColors(const string& path) {
	unique_ptr<rapidxml::file<>> pfdoc;
	try {
		pfdoc = make_unique<rapidxml::file<>>(path.c_str());
	}
	catch (std::exception e) {
		LOGE("error to parseRStrings %s file,error %s", path.c_str(), e.what());
		return;
	}
	if (pfdoc && pfdoc->size() > 0) {
		rapidxml::xml_document<> doc;// character type defaults to char
		doc.parse<0>(pfdoc->data());// 0 means default parse flags
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
		LOGE("error to parse resource string file %s",path.c_str());
	}
}

void UiManager::parseRStrings(const string& path) {
	unique_ptr<rapidxml::file<>> pfdoc;
	try {
		pfdoc = make_unique<rapidxml::file<>>(path.c_str());
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

shared_ptr<View> parseView(shared_ptr<View>& parent, rapidxml::xml_node<char>* pnode) {
	shared_ptr<View> pView;
	if (pnode != nullptr) {
		string viewName = pnode->name();
		pView = View::createView(viewName, nullptr);
		if (pView) {
			if (parent) {
				pView->setParent(parent);
				parent->addChild(pView);
			}

			auto attr = pnode->first_attribute();
			while (attr != nullptr) {
				string attrName = attr->name();
				
				auto it = View::gLayoutAttributeHandler.find(attrName);
				if (it != View::gLayoutAttributeHandler.end()) {
					it->second(pView, attr->value());
				}
				else {
					LOGD("there are no %s attributeHandler,please supplement", attrName.c_str());
				}
				
				attr = attr->next_attribute();
			}
		}
		auto child = pnode->first_node();
		auto sibling = pnode->next_sibling();
		if (child != nullptr) {
			parseView(pView, child);
		}
		if (sibling != nullptr) {
			parseView(parent, sibling);
		}
	}
	return pView;
}

//从一个xml文件里面加载一棵ui树，准备模仿Android的ui系统
shared_ptr<View> UiManager::loadFromFile(const string& filepath, int parentWidth, int parentHeight) {
	shared_ptr<View> rootView;
	//读取xml
	unique_ptr<rapidxml::file<>> pfdoc;
	try {
		pfdoc = make_unique<rapidxml::file<>>(filepath.c_str());
	}
	catch (std::exception e) {
		LOGE("error to loadFromFile %s file,error %s", filepath.c_str(), e.what());
		return gpViewNothing;
	}
	if (pfdoc->size() > 0) {
		auto pDoc = make_unique < rapidxml::xml_document<> >();// character type defaults to char
		pDoc->parse<0>(pfdoc->data());// 0 means default parse flags

		auto rootNode = pDoc->first_node();
		rootView = parseView(rootView, rootNode);
		//将有Id的控件搜集起来，以便查找
		rootView->getId2View(std::unique_ptr < std::unordered_map<std::string, std::shared_ptr<View>> >());
		rootView->calcRect(parentWidth, parentHeight);
	}
	return rootView;
}

//加载ui的布局文件
shared_ptr<View> UiManager::loadFromFile(const string& filepath) {
	return loadFromFile(filepath, static_cast<int>(mWindowWidth), static_cast<int>(mWindowHeight));
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
		mpUiTree->calcViewsRect(static_cast<int>(mWindowWidth), static_cast<int>(mWindowHeight));
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
			mpUiTree->mpRootView->mouseMove( x, y,false);
		}
	}
}

void UiManager::mouseLButtonDown(int x, int y) {
	if (mpUiTree) {
		if (mpUiTree->mpRootView) {
			mpUiTree->mpRootView->mouseLButtonDown( x, y,false);
		}
	}
}

void UiManager::mouseLButtonUp(int x, int y) {
	if (mpUiTree) {
		if (mpUiTree->mpRootView) {
			mpUiTree->mpRootView->mouseLButtonUp( x, y,false);
		}
	}
}

void UiManager::onDestroy() {
	UiRender::getInstance()->saveFonts();
}

bool Color::parseColor(const std::string& value, Color& color)
{
	const string temp = "@color/";
	auto index1 = value.find(temp);
	auto size = value.size();
	if ( index1 == 0) {
		auto colorName = value.substr(temp.size());
		color = UiManager::getColor(colorName);
		return true;
	}
	else if (value.empty() || size > 9 || value[0] != '#') {
		return false;
	}
	int index = 0;
	int relIndex = 1 + 2 * index;
	bool hasError = false;
	while (index < 4 && relIndex < size) {
		auto Rstr = value.substr(relIndex, 2);
		try {
			color[index] = (float)std::stoi(Rstr, nullptr, 16) / 255.0f;
		}
		catch (.../*const logic_error& e*/) {
			hasError = true;
			break;
		}
		++index;
		relIndex = 1 + 2 * index;
	}
	if (!hasError) {
		if (size == 7) {
			color.a = 1.0f;
		}
		else if (size == 5) {
			color.b = 0.0f;
			color.a = 1.0f;
		}
		else if (size == 3) {
			color.g = 0.0f;
			color.b = 0.0f;
			color.a = 1.0f;
		}
	}
	return !hasError;
}

Color& UiManager::getColor(const std::string& name) {
	static Color temp;
	auto it = gRColors.find(name);
	if (it != gRColors.end()) {
		return it->second;
	}
	else {
		LOGE("cannot find %s color in color resource", name.c_str());
		return temp;
	}
}

std::shared_ptr<Shape>& UiManager::getShape(const std::string& name) {
	static std::shared_ptr<Shape> temp;
	auto it = gRShapes.find(name);
	if (it != gRShapes.end()) {
		return it->second;
	}
	else {
		LOGE("cannot find %s shape in shape resource", name.c_str());
		return temp;
	}
}

std::shared_ptr<Texture>& UiManager::getTexture(const std::string& name) {
	return Material::getTexture(name);
}

std::shared_ptr<Background> UiManager::getBackground(const std::string& name) {
	auto it = gRBackground.find(name);
	if (it != gRBackground.end()) {
		return it->second;
	}
	else {
		LOGE("cannot find %s background in string resource", name.c_str());
		return std::shared_ptr<Background>();
	}
}

std::string& UiManager::getString(const std::string& name) {
	static std::string temp;
	auto it = gRStrings.find(name);
	if (it != gRStrings.end()) {
		return it->second;
	}
	else {
		LOGE("cannot find %s string in string resource",name.c_str());
		return temp;
	}
}

bool UiManager::initUi(int w, int h) {
	//加载ui string和color配置
	parseRStrings(gStringFile);
	parseRColors(gColorFile);
	//加载ui中需要用到的图片
	loadAllUiImage();
	loadAllShape();
	loadAllBackground();
	//初始化uirender
	UiRender::getInstance()->initUiRender();

	updateWidthHeight(static_cast<float > (w), static_cast<float>(h));
	return true;
}

void UiManager::draw() {
	if (mpUiTree) {
		//这个是将uitree绘制到纹理
		if (mpUiTree->draw()) {
			//把uitree渲染的结果拷贝到mpTexture里面，mpTexture已经设置给了uiRender
			//Fbo::blitFbo(mpUiTree->mFboForRender, mFboForCopy);
		}
		//这个是把纹理显示出来
		UiRender::getInstance()->drawUi();
	}
}