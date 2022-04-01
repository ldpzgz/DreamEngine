#include "MeshRoundedRectangle.h"

#include <glm/mat4x4.hpp>
#include <glm/mat4x3.hpp>
#include <glm/gtc/type_ptr.hpp> // value_ptr
#include "Log.h"
#include <vector>
#include <cmath>
static const int STEP = 32;
using namespace std;

MeshRoundedRectangle::MeshRoundedRectangle():
	MeshFilledRect()
{
	mMeshType = MeshType::RoundedRectangle;
}

MeshRoundedRectangle::~MeshRoundedRectangle() {

}

void MeshRoundedRectangle::loadMesh(float rightTopRadius,float leftTopRadius,float leftDownRadius,
		float rightDownRadius, int gradientAngle, float centerX,float centerY,float width, float height){
	mCenterX = centerX;
	mCenterY = centerY;
	mWidth = width;
	mHeight = height;
	std::vector<glm::vec2> texcoord;
	float angle = 0.0f;
	float halfWidth = width / 2.0f;
	float halfHeight = height / 2.0f;
	int maxRadius = std::min<float>(halfWidth, halfHeight);
	mPoints.clear();
	mPoints.reserve(STEP * 4 + 4);
	texcoord.reserve(STEP * 4 + 4);
	//中心点的处理
	mPoints.emplace_back(centerX, centerY, 0.0f);
	texcoord.emplace_back(centerX / mWidth, centerY / mHeight);
	glm::vec3 lastPoint{0.0f,0.0f,0.0f};
	glm::vec2 lastTex{ 0.0f,0.0f};
	float PI_1_2 = 3.1415926535897932f / 2.0f;
	//先从右上角开始
	if (rightTopRadius <= 0.0f) {
		mPoints.emplace_back(width, height, 0.0f);
		texcoord.emplace_back(1.0f, 1.0f);
		lastPoint = glm::vec3(width, height, 0.0f);
		lastTex = glm::vec2(1.0f, 1.0f);
	}
	else {
		if (rightTopRadius >= maxRadius)
			rightTopRadius = maxRadius;
		float texX = rightTopRadius / width;
		float texY = rightTopRadius / width;
		glm::vec2 texTranslate(1.0f - texX, 1.0f - texY);
		glm::vec3 translate(width - rightTopRadius, height - rightTopRadius, 0.0f);
		float tempAngle = angle;
		for (int i = 0; i < STEP; ++i) {
			mPoints.emplace_back(translate+ glm::vec3(rightTopRadius*cosf(tempAngle), rightTopRadius*sinf(tempAngle), 0.0f));
			texcoord.emplace_back(texTranslate + glm::vec2(texX*cosf(tempAngle), texY*sinf(tempAngle)));
			tempAngle = angle+i* PI_1_2 /(STEP-1);
		}
		angle += PI_1_2;
		lastPoint = translate + glm::vec3(rightTopRadius, 0.0f, 0.0f);
		lastTex = texTranslate + glm::vec2(texX, 0.0f);
	}

	if (gradientAngle == 0 || gradientAngle == 180) {
		mPoints.emplace_back(glm::vec3(mCenterX, mHeight, 0.0f));
		texcoord.emplace_back(glm::vec2(mCenterX / mWidth,1.0f));
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
		glm::vec2 texTranslate(texX, 1.0f - texY);
		glm::vec3 translate(leftTopRadius, height - leftTopRadius, 0.0f);
		float tempAngle = angle;
		for (int i = 0; i < STEP; ++i) {
			mPoints.emplace_back(translate + glm::vec3(leftTopRadius*cosf(tempAngle), leftTopRadius*sinf(tempAngle), 0.0f));
			texcoord.emplace_back(texTranslate + glm::vec2(texX*cosf(tempAngle), texY*sinf(tempAngle)));
			tempAngle = angle + i* PI_1_2 / (STEP - 1);
		}
		angle += PI_1_2;
	}

	if (gradientAngle == 90 || gradientAngle == 270) {
		mPoints.emplace_back(0.0f, mCenterY, 0.0f);
		texcoord.emplace_back(0.0f, mCenterY/mHeight);
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
		glm::vec2 texTranslate(texX, texY);
		glm::vec3 translate(leftDownRadius, leftDownRadius, 0.0f);
		float tempAngle = angle;
		for (int i = 0; i < STEP; ++i) {
			mPoints.emplace_back(translate + glm::vec3(leftDownRadius*cosf(tempAngle), leftDownRadius*sinf(tempAngle), 0.0f));
			texcoord.emplace_back(texTranslate + glm::vec2(texX*cosf(tempAngle), texY*sinf(tempAngle)));
			tempAngle = angle + i* PI_1_2 / (STEP - 1);
		}
		angle += PI_1_2;
	}

	if (gradientAngle == 0 || gradientAngle == 180) {
		mPoints.emplace_back(mCenterX, 0.0f, 0.0f);
		texcoord.emplace_back(mCenterX/mWidth,0.0f);
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
		glm::vec2 texTranslate(1.0f-texX, texY);
		glm::vec3 translate(width - rightDownRadius, rightDownRadius, 0.0f);
		float tempAngle = angle;
		for (int i = 0; i < STEP; ++i) {
			mPoints.emplace_back(translate + glm::vec3(rightDownRadius*cosf(tempAngle), rightDownRadius*sinf(tempAngle), 0.0f));
			texcoord.emplace_back(texTranslate + glm::vec2(texX*cosf(tempAngle), texY*sinf(tempAngle)));
			tempAngle = angle + i* PI_1_2 / (STEP - 1);
		}
		angle += PI_1_2;
	}

	if (gradientAngle == 90 || gradientAngle == 270) {
		if (gradientAngle == 0 || gradientAngle == 180) {
			mPoints.emplace_back(mWidth, mCenterY, 0.0f);
			texcoord.emplace_back(1.0f, mCenterY/mHeight);
		}
	}

	mPoints.emplace_back(lastPoint);
	texcoord.emplace_back(lastTex);
	int numOfVertex = mPoints.size();
	bool b = createBufferObject((float*)mPoints.data(), 3*sizeof(float)* numOfVertex, 
		numOfVertex,
		nullptr, 0,
		(float*)texcoord.data(),texcoord.size()*sizeof(glm::vec2));
	if (!b)
	{
		LOGD("error to load MeshRoundedRectangle\n");
	}
}
		
void MeshRoundedRectangle::loadMesh(float radius, int gradientAngle, float centerX,float centerY,float width, float height){
	loadMesh(radius, radius, radius, radius, gradientAngle,centerX,centerY,width,height);
}