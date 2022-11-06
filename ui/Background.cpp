#include "Background.h"
#include "BackgroundStyle.h"
#include "core/MeshFilledRect.h"
#include "Shape.h"
using namespace std;

Background::Background() {
	mpNormalStyle = std::make_unique<BackgroundStyle>();
	mpNormalStyle->mpShape = std::make_shared<Shape>();
}

