#include "TextGpuRender.h"
#include "../MeshBezier.h"
void TextGpuRender::init() {
	mpMesh = make_shared<Mesh>(MeshType::MESH_DIY);
	mpUniformColorMaterial = Material::getMaterial("posUniformColor");
	mpTextGpuRenderMaterial = Material::getMaterial("textGpuRender");
	mpMultisampleTexture = Material::getTexture("tgrTexture");
	mpFirstTexture = Material::createTexture("tgrFirst", 512, 512, nullptr, GL_LUMINANCE);

	mpMesh->setMaterial(mpUniformColorMaterial);
	mpUniformColorMaterial->setUniformColor(1.0f / 255.0f, 0.0f, 0.0f, 0.0f);
	mpFbo = make_shared<Fbo>();
	mpFbo->attachColorTexture(mpFirstTexture);
	mpFbo->setDepthTest(false);
	mpFbo->setBlend(true);
	mpFbo->setBlendValue(GL_ONE, GL_ONE, GL_ONE, GL_ONE, GL_FUNC_ADD, GL_FUNC_ADD);
	mPos.emplace_back(-1.0f, -1.0f, 0.0f);
}

int TextGpuRender::moveTo(const FT_Vector* vec, void* userData) {
	if (userData != nullptr && vec != nullptr) {
		TextGpuRender* pRender = (TextGpuRender*)userData;
		pRender->mPos.emplace_back(vec->x, vec->y, 0.0f);
		return 0;
	}
	return 1;
}

int TextGpuRender::lineTo(const FT_Vector* vec, void* userData) {
	if (userData != nullptr && vec!=nullptr) {
		TextGpuRender* pRender = (TextGpuRender*)userData;
		pRender->mPos.emplace_back(vec->x, vec->y, 0.0f);
		int size = pRender->mPos.size();
		pRender->mIndex.emplace_back(0, size - 2, size - 1);
		return 0;
	}
	return 1;
}

int TextGpuRender::conicTo(const FT_Vector* control,const FT_Vector* to, void* userData) {
	if (userData != nullptr && control != nullptr && to != nullptr) {
		TextGpuRender* pRender = (TextGpuRender*)userData;
		Vec3& first = pRender->mPos.back();
		std::vector<Vec3> points;
		std::vector<Vec3> result;
		points.emplace_back(first);
		points.emplace_back(control->x, control->y,0.0f);
		points.emplace_back(to->x, to->y, 0.0f);
		MeshBezier::getBezierPoints(points, 6, result);
		for (int i = 1; i < 6; ++i) {
			pRender->mPos.emplace_back(result[i]);
			int size = pRender->mPos.size();
			pRender->mIndex.emplace_back(0, size - 2, size - 1);
		}
		return 0;
	}
	return 1;
}

int TextGpuRender::cubicTo(const FT_Vector* control1,const FT_Vector* control2,const FT_Vector* to, void* userData) {
	if (userData != nullptr && control1 != nullptr && control2 != nullptr && to != nullptr) {
		TextGpuRender* pRender = (TextGpuRender*)userData;
		Vec3& first = pRender->mPos.back();
		std::vector<Vec3> points;
		std::vector<Vec3> result;
		points.emplace_back(first);
		points.emplace_back(control1->x, control1->y, 0.0f);
		points.emplace_back(control2->x, control2->y, 0.0f);
		points.emplace_back(to->x, to->y, 0.0f);
		MeshBezier::getBezierPoints(points, 6, result);
		for (int i = 1; i < 6; ++i) {
			pRender->mPos.emplace_back(result[i]);
			int size = pRender->mPos.size();
			pRender->mIndex.emplace_back(0, size - 2, size - 1);
		}
		return 0;
	}
	return 1;
}

void TextGpuRender::generateMesh() {
	if (mpMesh) {
		mpMesh->updataPos((GLfloat*)mPos.data(), 0,sizeof(Vec3) * mPos.size(), mPos.size());
		mpMesh->updataIndex((GLuint*)mIndex.data(), 0,sizeof(Vec3ui) * mIndex.size());
	}
	else {
		LOGE("ERROR TextGpuRender::generateMesh should be called after init()");
	}
}

void TextGpuRender::render(glm::mat4 modelMatrix) {
	if (mpFbo && mpMesh) {
		mpFbo->render([this, &modelMatrix]() {
			mpMesh->render(mProjMatrix * modelMatrix);
			});
	}
	else {
		LOGE("ERROR to call TextGpuRender::render the mpFbo or mpMesh is null");
	}
}

void TextGpuRender::drawTextView(TextView* tv) {

}