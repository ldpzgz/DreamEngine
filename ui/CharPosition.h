#pragma once

#include <glm/mat4x4.hpp>

//一个字符的渲染信息
class CharPosition {
public:
	CharPosition() :
		matrix(1.0),
		texMatrix(1.0) {

	}
	CharPosition(const CharPosition& cp) {
		matrix = cp.matrix;
		texMatrix = cp.texMatrix;
	}
	glm::mat4 matrix;//描述了字符的位置，缩放，旋转等信息，这个矩阵负责把一个0，1的矩形绘制到目的地。
	glm::mat4 texMatrix;//这个矩阵负责把一个0，1的纹理坐标，变换到该字符所占用的纹理区域
};

