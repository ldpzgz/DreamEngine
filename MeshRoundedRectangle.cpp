#include "MeshRoundedRectangle.h"
#include <vector>
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp> // vec3, vec4, ivec4, mat4
#include <glm/mat4x4.hpp>
#include <glm/mat4x3.hpp>
#include <glm/gtc/type_ptr.hpp> // value_ptr
#include <cmath>
static const int STEP = 32;
using namespace std;

MeshRoundedRectangle::MeshRoundedRectangle():
	MeshFilledRect()
{
	mMeshType = MeshType::MESH_Rounded_Rectangle;
}

MeshRoundedRectangle::~MeshRoundedRectangle() {

}

void MeshRoundedRectangle::loadMesh(float rightTopRadius,float leftTopRadius,float leftDownRadius,
		float rightDownRadius,float centerX,float centerY,float width, float height){
	mCenterX = centerX;
	mCenterY = centerY;
	mWidth = width;
	mHeight = height;
	std::vector<Vec2> texcoord;
	float angle = 0.0f;
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;
	int maxRadius = std::min<float>(halfWidth, halfHeight);
	mPoints.clear();
	mPoints.reserve(STEP * 4 + 2);
	texcoord.reserve(STEP * 4 + 2);
	//中心点的处理
	mPoints.emplace_back(centerX, centerY, 0.0f);
	texcoord.emplace_back(centerX / mWidth, centerY / mHeight);
	Vec3 lastPoint;
	Vec2 lastTex;
	float PI_1_2 = 3.1415926535897932f / 2.0f;
	//先从右上角开始
	if (rightTopRadius <= 0.0f) {
		mPoints.emplace_back(width, height, 0.0f);
		texcoord.emplace_back(1.0f, 1.0f);
		lastPoint = Vec3(width, height, 0.0f);
		lastTex = Vec2(1.0f, 1.0f);
	}
	else {
		if (rightTopRadius >= maxRadius)
			rightTopRadius = maxRadius;
		float texX = rightTopRadius / width;
		float texY = rightTopRadius / width;
		Vec2 texTranslate(1.0f - texX, 1.0f - texY);
		Vec3 translate(width - rightTopRadius, height - rightTopRadius, 0.0f);
		float tempAngle = angle;
		for (int i = 0; i < STEP; ++i) {
			mPoints.emplace_back(translate+ Vec3(rightTopRadius*cosf(tempAngle), rightTopRadius*sinf(tempAngle), 0.0f));
			texcoord.emplace_back(texTranslate + Vec2(texX*cosf(tempAngle), texY*sinf(tempAngle)));
			tempAngle = angle+i* PI_1_2 /(STEP-1);
		}
		angle += PI_1_2;
		lastPoint = translate + Vec3(rightTopRadius, 0.0f, 0.0f);
		lastTex = texTranslate + Vec2(texX, 0.0f);
	}

	if (leftTopRadius <= 0.0f) {
		mPoints.emplace_back(0.0f, height, 0.0f);
		texcoord.emplace_back(0.0f, 1.0f);
	}
	else {
		if (leftTopRadius >= maxRadius)
			leftTopRadius = maxRadius;
		float texX = leftTopRadius / width;
		float texY = leftTopRadius / width;
		Vec2 texTranslate(texX, 1.0f - texY);
		Vec3 translate(leftTopRadius, height - leftTopRadius, 0.0f);
		float tempAngle = angle;
		for (int i = 0; i < STEP; ++i) {
			mPoints.emplace_back(translate + Vec3(leftTopRadius*cosf(tempAngle), leftTopRadius*sinf(tempAngle), 0.0f));
			texcoord.emplace_back(texTranslate + Vec2(texX*cosf(tempAngle), texY*sinf(tempAngle)));
			tempAngle = angle + i* PI_1_2 / (STEP - 1);
		}
		angle += PI_1_2;
	}

	if (leftDownRadius <= 0.0f) {
		mPoints.emplace_back(0.0f, 0.0f, 0.0f);
		texcoord.emplace_back(0.0f, 0.0f);
	}
	else {
		if (leftDownRadius >= maxRadius)
			leftDownRadius = maxRadius;
		float texX = leftDownRadius / width;
		float texY = leftDownRadius / width;
		Vec2 texTranslate(texX, texY);
		Vec3 translate(leftDownRadius, leftDownRadius, 0.0f);
		float tempAngle = angle;
		for (int i = 0; i < STEP; ++i) {
			mPoints.emplace_back(translate + Vec3(leftDownRadius*cosf(tempAngle), leftDownRadius*sinf(tempAngle), 0.0f));
			texcoord.emplace_back(texTranslate + Vec2(texX*cosf(tempAngle), texY*sinf(tempAngle)));
			tempAngle = angle + i* PI_1_2 / (STEP - 1);
		}
		angle += PI_1_2;
	}

	if (rightDownRadius <= 0.0f) {
		mPoints.emplace_back(width, 0.0f, 0.0f);
		texcoord.emplace_back(1.0f, 0.0f);
	}
	else {
		if (rightDownRadius >= maxRadius)
			rightDownRadius = maxRadius;
		float texX = leftDownRadius / width;
		float texY = leftDownRadius / width;
		Vec2 texTranslate(1.0f-texX, texY);
		Vec3 translate(width - rightDownRadius, rightDownRadius, 0.0f);
		float tempAngle = angle;
		for (int i = 0; i < STEP; ++i) {
			mPoints.emplace_back(translate + Vec3(rightDownRadius*cosf(tempAngle), rightDownRadius*sinf(tempAngle), 0.0f));
			texcoord.emplace_back(texTranslate + Vec2(texX*cosf(tempAngle), texY*sinf(tempAngle)));
			tempAngle = angle + i* PI_1_2 / (STEP - 1);
		}
		angle += PI_1_2;
	}

	mPoints.emplace_back(lastPoint);
	texcoord.emplace_back(lastTex);

	bool b = createBufferObject((float*)mPoints.data(), 3*sizeof(float)*mPoints.size(), nullptr, 0,
		(float*)texcoord.data(),2*sizeof(float)*texcoord.size());
	if (!b)
	{
		LOGD("error to load MeshRoundedRectangle\n");
	}
}
		
void MeshRoundedRectangle::loadMesh(float radius,float centerX,float centerY,float width, float height){
	loadMesh(radius, radius, radius, radius, centerX,centerY,width,height);
}