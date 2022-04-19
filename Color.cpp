#include "Color.h"
#include <string>
#include "Resource.h"
using namespace std;
bool Color::parseColor(const std::string& value, Color& color)
{
	const string temp = "@color/";
	auto index1 = value.find(temp);
	auto size = value.size();
	if (index1 == 0) {
		auto colorName = value.substr(temp.size());
		color = Resource::getInstance().getColor(colorName);
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