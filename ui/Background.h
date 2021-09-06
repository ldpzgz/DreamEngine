#ifndef _BACKGROUND_H_
#define _BACKGROUND_H_
#include "../Texture.h"
#include "../Mesh.h"
#include "Shape.h"
#include <memory>
/*
* 绘制shape需要的mesh，texture
*/
class Background {
public:
	std::shared_ptr<Shape> mpShape;
	std::shared_ptr<Texture> mpTexture;
	std::shared_ptr<Mesh> mpMesh;//承载shape的mesh
	std::shared_ptr<Mesh> mpStrokeMesh;//承载边框的mesh
};

#endif