#ifndef _TEXT_GPU_RENDER_H_
#define _TEXT_GPU_RENDER_H_
#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_GLYPH_H
#include FT_OUTLINE_H 
#include "../Rect.h"
#include "../Mesh.h"
#include "../Material.h"
#include "../Fbo.h"
#include <vector>
#include <memory>
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/mat4x4.hpp>
#include <glm/mat3x3.hpp>
// Include all GLM extensions
#include <glm/ext.hpp> // perspective, translate, rotate
#include <glm/gtx/matrix_transform_2d.hpp>
#include "TextView.h"
/*
* 渲染步骤如下：
* 1 从free type得到的字符的三角形mesh，设置posUniformColor材质，指定color为1/255。
*   开启blend，设置好相应的blend参数，渲染字符mesh到一张普通纹理上。
* 2 搞一个MeshRect,设置设置一个专用的material，根据颜色值的奇偶来点亮字符，绑定上面渲染好的纹理，渲染这个rect到多重采样
* 
* 3 用MeshRect，设置一个访问多重采样纹理的material，根据TextView的位置信息，将字符渲染到对于的位置上。
* 
* 一个字符需要渲染三遍
*/
class TextGpuRender
{
public:
	TextGpuRender() = default;
	~TextGpuRender() = default;

	void init();

	void clear() {
		if (mpMesh) {
			mpMesh->unLoadMesh();
		}
		mPos.clear();
		mIndex.clear();
	}

	void setRenderTexture(const TextureP& pTex) {
		if (mpFbo && pTex) {
			mpFbo->attachColorTexture(pTex);
			mProjMatrix = glm::ortho(0, pTex->getWidth()-1, 0, pTex->getHeight()-1);
		}
		else {
			LOGE("ERROR to call setRenderTexture fbo or ptex is null");
		}
	}
	//当字形转换为三角形之后，调用这个函数生成mesh
	void generateMesh();

	void drawTextView(TextView* tv);

	void render(glm::mat4 modelMatrix);

	static int moveTo(const FT_Vector* vec,void* userData);
	static int lineTo(const FT_Vector* vec, void* userData);
	static int conicTo(const FT_Vector* control1, const FT_Vector* to, void* userData);
	static int cubicTo(const FT_Vector* control1, const FT_Vector* control2, const FT_Vector* to, void* userData);
	MeshP mpMesh;
	MaterialP mpUniformColorMaterial;
	MaterialP mpTextGpuRenderMaterial;
	TextureP mpFirstTexture;
	TextureP mpMultisampleTexture;
	std::shared_ptr<Fbo> mpFbo;
	std::vector<Vec3> mPos;
	std::vector<Vec3ui> mIndex;
	glm::mat4 mProjMatrix{ 1.0f };
};
using TextGpuRenderP = std::shared_ptr<TextGpuRender>;
#endif

