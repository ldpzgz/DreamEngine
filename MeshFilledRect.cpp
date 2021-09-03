#include "MeshFilledRect.h"

MeshFilledRect::MeshFilledRect():
	Mesh()
{
	mMeshType = MeshType::MESH_FilledRect;
}

MeshFilledRect::~MeshFilledRect() {
	mPoints.clear();
}

void MeshFilledRect::setColorData(float angle, const Color& startColor, const Color& endColor, const Color& centerColor) {
	vector<Vec4> colors;
	Vec4 start(startColor.r, startColor.g, startColor.b, startColor.a);
	Vec4 end(endColor.r, endColor.g, endColor.b, endColor.a);
	Vec4 center(centerColor.r, centerColor.g, centerColor.b, centerColor.a);

	if (angle == 0) {
		if (centerColor.isZero()) {
			auto size = mPoints.size();
			for (int i = 0; i < size; ++i) {
				float endFactor = mPoints[i].x / mWidth;
				float startFactor = 1 - endFactor;
				colors.emplace_back(startFactor*start + endFactor*end);
			}
		}
		else {
			colors.emplace_back(center);
			auto size = mPoints.size();
			for (int i = 1; i < size; ++i) {
				int x = mPoints[i].x;
				if (x > mCenterX) {
					float endFactor = (x - mCenterX) / (mWidth - mCenterX);
					float startFactor = 1 - endFactor;
					colors.emplace_back(startFactor*center + endFactor*end);
				}
				else {
					float endFactor = x / mCenterX;
					float startFactor = 1 - endFactor;
					colors.emplace_back(startFactor*start + endFactor*center);
				}
			}
		}
	}
	else if (angle == 90) {
		if (centerColor.isZero()) {
			auto size = mPoints.size();
			for (int i = 0; i < size; ++i) {
				float endFactor = mPoints[i].y / mHeight;
				float startFactor = 1 - endFactor;
				colors.emplace_back(startFactor*start + endFactor*end);
			}
		}
		else {
			colors.emplace_back(center);
			auto size = mPoints.size();
			for (int i = 1; i < size; ++i) {
				int y = mPoints[i].y;
				if (y > mCenterY) {
					float endFactor = (y - mCenterY) / (mHeight - mCenterY);
					float startFactor = 1 - endFactor;
					colors.emplace_back(startFactor*center + endFactor*end);
				}
				else {
					float endFactor = y / mCenterY;
					float startFactor = 1 - endFactor;
					colors.emplace_back(startFactor*start + endFactor*center);
				}
			}
		}
	}
	else if (angle == 180) {
		if (centerColor.isZero()) {
			auto size = mPoints.size();
			for (int i = 0; i < size; ++i) {
				float endFactor = mPoints[i].x / mWidth;
				float startFactor = 1 - endFactor;
				colors.emplace_back(endFactor*start + startFactor*end);
			}
		}
		else {
			colors.emplace_back(center);
			auto size = mPoints.size();
			for (int i = 1; i < size; ++i) {
				int x = mPoints[i].x;
				if (x > mCenterX) {
					float endFactor = (x - mCenterX) / (mWidth - mCenterX);
					float startFactor = 1 - endFactor;
					colors.emplace_back(endFactor*center + startFactor*end);
				}
				else {
					float endFactor = x / mCenterX;
					float startFactor = 1 - endFactor;
					colors.emplace_back(endFactor*start + startFactor*center);
				}
			}
		}
	}
	else if (angle == 270) {
		if (centerColor.isZero()) {
			auto size = mPoints.size();
			for (int i = 0; i < size; ++i) {
				float endFactor = mPoints[i].y / mHeight;
				float startFactor = 1 - endFactor;
				colors.emplace_back(endFactor*start + startFactor*end);
			}
		}
		else {
			colors.emplace_back(center);
			auto size = mPoints.size();
			for (int i = 1; i < size; ++i) {
				int y = mPoints[i].y;
				if (y > mCenterY) {
					float endFactor = (y - mCenterY) / (mHeight - mCenterY);
					float startFactor = 1 - endFactor;
					colors.emplace_back(endFactor*center + startFactor*end);
				}
				else {
					float endFactor = y / mCenterY;
					float startFactor = 1 - endFactor;
					colors.emplace_back(endFactor*start + startFactor*center);
				}
			}
		}
	}
	else {
		LOGD("ERROR not support gradient angle %f in shape",angle);
	}

	Mesh::setColorData((float*)colors.data(), 4 * sizeof(float)*colors.size());
}

void MeshFilledRect::loadMesh(float width, float height, float centerX, float centerY) {
	mCenterX = centerX;
	mCenterY = centerY;
	mWidth = width;
	mHeight = height;
	mCounts = 6;
	mPoints.reserve(mCounts);
	float tex[2 * 6];
	tex[0] = mCenterX / mWidth;
	tex[1] = mCenterY / mHeight;
	mPoints.emplace_back(mCenterX, mCenterY, 0.0f);

	tex[2] = 1.0f;
	tex[3] = 1.0f;
	mPoints.emplace_back(mWidth, mHeight, 0.0f);

	tex[4] = 0.0f;
	tex[5] = 1.0f;
	mPoints.emplace_back(0.0f, mHeight, 0.0f);

	tex[6] = 0.0f;
	tex[7] = 0.0f;
	mPoints.emplace_back(0.0f, 0.0f, 0.0f);

	tex[8] = 1.0f;
	tex[9] = 0.0f;
	mPoints.emplace_back(mWidth, 0.0f, 0.0f);

	tex[10] = 1.0f;
	tex[11] = 1.0f;
	mPoints.emplace_back(mWidth, mHeight, 0.0f);
		
	bool b = createBufferObject((float*)mPoints.data(), mCounts * 3 * sizeof(GLfloat),nullptr,0,tex,sizeof(GLfloat)*mCounts*2);
	if (!b)
	{
		LOGD("error to load MeshFilledRect data\n");
	}
}

void MeshFilledRect::draw(int posloc, int texloc, int norloc, int colorloc) {
	if (mbFilled) {
		drawTrangleFan(posloc, texloc, norloc, colorloc);
	}
	else {
		drawLineStrip(posloc);
	}
}

void MeshFilledRect::drawLineStrip(int posloc) {
	if (createVaoIfNeed(posloc)) {
		glBindVertexArray(mVAO);
		if (mposLocation >= 0) {
			glBindBuffer(GL_ARRAY_BUFFER, mPosVbo);
			//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mIndexVbo);
			glEnableVertexAttribArray(posloc);
			glVertexAttribPointer(posloc, 3, GL_FLOAT, GL_FALSE, 0,(void*)(3*sizeof(GLfloat)));
		}
		else {

		}
		glBindVertexArray(0);
	}

	glBindVertexArray(mVAO);
	glLineWidth(mLineWidth);
	glDrawArrays(GL_LINE_STRIP, 0, mCounts-1);
	glBindVertexArray(0);
}