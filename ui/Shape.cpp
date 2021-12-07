#include <stdexcept>
#include "Shape.h"
using namespace std;

std::unordered_map < std::string, std::function<void(const shared_ptr<Shape>&, const std::string&)>> Shape::gShapeAttributeHandler{
	{"cornerRadius",Shape::cornerRadiusHandler },
	{ "leftTopRadius",Shape::cornerLeftTopRadiusHandler },
	{ "leftBottomRadius",Shape::cornerLeftBottomRadiusHandler },
	{ "rightTopRadius",Shape::cornerRightTopRadiusHandler },
	{ "rightBottomRadius",Shape::cornerRightBottomRadiusHandler },

	{ "angle",Shape::gradientAngleHandler },
	{ "centerX",Shape::gradientCenterXHandler },
	{ "centerY",Shape::gradientCenterYHandler },
	{ "centerColor",Shape::gradientCenterColorHandler },
	{ "startColor",Shape::gradientStartColorHandler },
	{ "endColor",Shape::gradientEndColorHandler },
	{ "gradientRadius",Shape::gradientRadiusHandler },
	{ "gradientType",Shape::gradientTypeHandler },

	{ "paddingLeft",Shape::paddingLeftHandler },
	{ "paddingRight",Shape::paddingRightHandler },
	{ "paddingBottom",Shape::paddingBottomHandler},
	{ "paddingTop",Shape::paddingTopHandler },

	{ "ovalWidth",Shape::sizeWidthHandler },
	{ "ovalHeight",Shape::sizeHeightHandler },

	{"solidColor",Shape::solidColorHandler },

	{ "strokeWidth",Shape::strokeWidthHandler },
	{ "strokeColor",Shape::strokeColorHandler },
	{ "dashWidth",Shape::strokeDashWidthHandler },
	{ "dashGap",Shape::strokeDashGapHandler },
};

void Shape::cornerRadiusHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	try {
		if (shape) {
			shape->setCornerRadius(stoi(value));
		}
	}
	catch (const logic_error& e) {
		LOGE("error to parse cornerRadius value %s", value.c_str());
	}
}
void Shape::cornerLeftTopRadiusHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	try {
		if (shape) {
			shape->setCornerLeftTopRadius(stoi(value));
		}
	}
	catch (const logic_error& e) {
		LOGE("error to parse cornerLeftTopRadius value %s", value.c_str());
	}

}
void Shape::cornerLeftBottomRadiusHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	try {
		if (shape) {
			shape->setCornerLeftBottomRadius(stoi(value));
		}
	}
	catch (const logic_error& e) {
		LOGE("error to parse cornerLeftBottomRadius value %s", value.c_str());
	}
}
void Shape::cornerRightTopRadiusHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	try {
		if (shape) {
			shape->setCornerRightTopRadius(stoi(value));
		}
	}
	catch (const logic_error& e) {
		LOGE("error to parse cornerRightTopRadius value %s", value.c_str());
	}
}
void Shape::cornerRightBottomRadiusHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	try {
		if (shape) {
			shape->setCornerRightBottomRadius(stoi(value));
		}
	}
	catch (const logic_error& e) {
		LOGE("error to parse cornerRightBottomRadius value %s", value.c_str());
	}
}

void Shape::gradientAngleHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	try {
		if (shape) {
			shape->setGradientAngle(stoi(value));
		}
	}
	catch (const logic_error& e) {
		LOGE("error to parse gradientAngle value %s", value.c_str());
	}
}
void Shape::gradientCenterXHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	try {
		if (shape) {
			shape->setGradientCenterX(stof(value));
		}
	}
	catch (const logic_error& e) {
		LOGE("error to parse gradientCenterX value %s", value.c_str());
	}
}
void Shape::gradientCenterYHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	try {
		if (shape) {
			shape->setGradientCenterY(stof(value));
		}
	}
	catch (const logic_error& e) {
		LOGE("error to parse gradientCenterY value %s", value.c_str());
	}
}
void Shape::gradientCenterColorHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	if (shape) {
		auto& color = shape->getGradientCenterColor();
		if (!Color::parseColor(value, color)) {
			LOGE("error to parse gradientCernterColor value %s", value.c_str());
		}
	}
}
void Shape::gradientStartColorHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	if (shape) {
		auto& color = shape->getGradientStartColor();
		if (!Color::parseColor(value, color)) {
			LOGE("error to parse gradientStartColor value %s", value.c_str());
		}
	}
}
void Shape::gradientEndColorHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	if (shape) {
		auto& color = shape->getGradientEndColor();
		if (!Color::parseColor(value, color)) {
			LOGE("error to parse gradientEndColor value %s", value.c_str());
		}
	}
}
void Shape::gradientRadiusHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	if (shape) {
		try {
			shape->setGradientRadius(stof(value));
		}
		catch (const logic_error& e) {
			LOGE("error to parse gradientRadius value %s", value.c_str());
		}
	}
}
void Shape::gradientTypeHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	if (shape) {
		shape->setGradientType(value);
	}
}

void Shape::paddingHandler(const std::shared_ptr<Shape>& shape, const std::string& value) {
	if (shape) {
		try {
			shape->setPadding(stoi(value));
		}
		catch (const logic_error& e) {
			LOGE("error to parse padding value %s", value.c_str());
		}
	}
}

void Shape::paddingLeftHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	if (shape) {
		try {
			shape->setPaddingLeft(stoi(value));
		}
		catch (const logic_error& e) {
			LOGE("error to parse paddingLeft value %s", value.c_str());
		}
	}
}
void Shape::paddingRightHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	if (shape) {
		try {
			shape->setPaddingRight(stoi(value));
		}
		catch (const logic_error& e) {
			LOGE("error to parse paddingRight value %s", value.c_str());
		}
	}
}

void Shape::paddingBottomHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	if (shape) {
		try {
			shape->setPaddingBottom(stoi(value));
		}
		catch (const logic_error& e) {
			LOGE("error to parse paddingBottom value %s", value.c_str());
		}
	}
}
void Shape::paddingTopHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	if (shape) {
		try {
			shape->setPaddingTop(stoi(value));
		}
		catch (const logic_error& e) {
			LOGE("error to parse paddingTop value %s", value.c_str());
		}
	}
}

void Shape::sizeWidthHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	if (shape) {
		try {
			shape->setOvalWidth(stof(value));
		}
		catch (const logic_error& e) {
			LOGE("error to parse sizeWidth value %s", value.c_str());
		}
	}
}

void Shape::sizeHeightHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	if (shape) {
		try {
			shape->setOvalHeight(stof(value));
		}
		catch (const logic_error& e) {
			LOGE("error to parse sizeHeight value %s", value.c_str());
		}
	}
}

void Shape::solidColorHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	if (shape) {
		auto& color = shape->getSolidColor();
		if (!Color::parseColor(value, color)) {
			LOGE("error to parse solidColor value %s", value.c_str());
		}
	}
}

void Shape::strokeWidthHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	if (shape) {
		try {
			shape->setSrokeWidth(stoi(value));
		}
		catch (const logic_error& e) {
			LOGE("error to parse strokeWidth value %s", value.c_str());
		}
	}
}

void Shape::strokeColorHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	if (shape) {
		auto& color = shape->getSrokeColor();
		if (!Color::parseColor(value, color)) {
			LOGE("error to parse strokeColor value %s", value.c_str());
		}
	}
}

void Shape::strokeDashWidthHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	if (shape) {
		try {
			shape->setSrokeDashWidth(stoi(value));
		}
		catch (const logic_error& e) {
			LOGE("error to parse strokeDashWidth value %s", value.c_str());
		}
	}
}

void Shape::strokeDashGapHandler(const shared_ptr<Shape>& shape, const std::string& value) {
	if (shape) {
		try {
			shape->setSrokeDashGap(stoi(value));
		}
		catch (const logic_error& e) {
			LOGE("error to parse strokeDashGap value %s", value.c_str());
		}
	}
}