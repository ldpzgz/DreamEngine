#include "Background.h"
#include <rapidxml.hpp>
#include <rapidxml_utils.hpp>  //rapidxml::file
#include <rapidxml_print.hpp>  //rapidxml::print
#include "../Utils.h"

using namespace std;
void textureHandler(std::unique_ptr<Background::BackgroundStyle>& pStyle , const string& value) {

}
void startColorHandler(std::unique_ptr<Background::BackgroundStyle>& pStyle, const string& value) {

}
void centerColorHandler(std::unique_ptr<Background::BackgroundStyle>& pStyle, const string& value) {

}
void endColorHandler(std::unique_ptr<Background::BackgroundStyle>& pStyle, const string& value) {

}
void solidColorHandler(std::unique_ptr<Background::BackgroundStyle>& pStyle, const string& value) {

}
unordered_map<const string&, std::function<void(std::unique_ptr<Background::BackgroundStyle>&, const string&)>> gStyleHandlers{
	{"texture",textureHandler},
	{"solidColor",solidColorHandler},
	{"startColor",startColorHandler},
	{"centerColor",centerColorHandler},
	{"endColor",endColorHandler},
};

void attributeHandler(std::unique_ptr<Background::BackgroundStyle>& pStyle, rapidxml::xml_attribute<char>* pAttribute) {
	auto endIt = gStyleHandlers.end();
	while (pAttribute) {
		string name = pAttribute->name();
		string value = pAttribute->value();
		auto it = gStyleHandlers.find(name);
		if (it != endIt) {
			it->second(pStyle, value);
		}
		else {
			LOGE("ERROR to parse background xml,the attribute %s is illeague",name.c_str());
			return;
		}
		pAttribute = pAttribute->next_attribute();
	}
}
void Background::nodeHandler(rapidxml::xml_node<char>* pNode) {
	if (pNode) {
		std::string name = pNode->name();
		if (name == "normal") {
			mpNormalStyle = std::make_unique<Background::BackgroundStyle>();
			attributeHandler(mpNormalStyle, pNode->first_attribute());
		}
		else if (name == "pushed") {
			mpPushedStyle = std::make_unique<Background::BackgroundStyle>();
			attributeHandler(mpPushedStyle, pNode->first_attribute());
		}
		else if (name == "hover") {
			mpHoverStyle = std::make_unique<Background::BackgroundStyle>();
			attributeHandler(mpHoverStyle, pNode->first_attribute());
		}
		else if (name == "disabled") {
			mpDisabledStyle = std::make_unique<Background::BackgroundStyle>();
			attributeHandler(mpDisabledStyle, pNode->first_attribute());
		}
	}
}
