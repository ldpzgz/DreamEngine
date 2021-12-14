#include "MeshFilledRect.h"

MeshFilledRect::MeshFilledRect():
	Mesh()
{
	mMeshType = MeshType::MESH_FilledRect;
}

MeshFilledRect::~MeshFilledRect() {
	mPoints.clear();
	if (!mExtraColorVbos.empty()) {
		glDeleteBuffers(mExtraColorVbos.size(), mExtraColorVbos.data());
	}
}

unsigned int MeshFilledRect::createAColorData(float angle, const Color& startColor, const Color& endColor, const Color& centerColor) {
	vector<Vec4> colors;
	Vec4 start(startColor.r, startColor.g, startColor.b, startColor.a);
	Vec4 end(endColor.r, endColor.g, endColor.b, endColor.a);
	Vec4 center(centerColor.r, centerColor.g, centerColor.b, centerColor.a);
	if (centerColor.isZero()) {
		center = 0.5f * start + 0.5f * end;
	}
	if (angle == 0) {
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
	else if (angle == 90) {
		
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
	else if (angle == 180) {
		
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
	else if (angle == 270) {
		
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
	else {
		LOGD("ERROR not support gradient angle %f in shape",angle);
	}

	
	GLuint colorVbo=0;
	int colorSize = 4 * sizeof(float) * colors.size();
	glGenBuffers(1, &colorVbo);
	glBindBuffer(GL_ARRAY_BUFFER, colorVbo);
	glBufferData(GL_ARRAY_BUFFER, colorSize, (float*)colors.data(), GL_STATIC_DRAW);
	if (mColorVbo == 0) {
		mColorVbo = colorVbo;
		mColorByteSize = colorSize;
	}
	else {
		mExtraColorVbos.emplace_back(colorVbo);
	}
	return colorVbo;
}

void MeshFilledRect::setColorVbo(unsigned int vbo) {
	mColorVbo = vbo;
}

void MeshFilledRect::loadMesh(float width, float height, int gradientAngle, float centerX, float centerY) {
	mCenterX = centerX;
	mCenterY = centerY;
	mWidth = width;
	mHeight = height;
	int numOfVertex = 8;
	float tex[2 * 8];
	int index = 0;
	tex[index++] = mCenterX / mWidth;
	tex[index++] = mCenterY / mHeight;
	mPoints.emplace_back(mCenterX, mCenterY, 0.0f);

	tex[index++] = 1.0f;
	tex[index++] = 1.0f;
	mPoints.emplace_back(mWidth, mHeight, 0.0f);
	if (gradientAngle == 0 || gradientAngle == 180) {
		tex[index++] = tex[0];
		tex[index++] = 1.0f;
		mPoints.emplace_back(mCenterX, mHeight, 0.0f);
	}

	tex[index++] = 0.0f;
	tex[index++] = 1.0f;
	mPoints.emplace_back(0.0f, mHeight, 0.0f);

	if (gradientAngle == 90 || gradientAngle == 270) {
		tex[index++] = 0;
		tex[index++] = tex[1];
		mPoints.emplace_back(0.0f, mCenterY, 0.0f);
	}

	tex[index++] = 0.0f;
	tex[index++] = 0.0f;
	mPoints.emplace_back(0.0f, 0.0f, 0.0f);

	if (gradientAngle == 0 || gradientAngle == 180) {
		tex[index++] = tex[0];
		tex[index++] = 0.0f;
		mPoints.emplace_back(mCenterX, 0.0f, 0.0f);
	}

	tex[index++] = 1.0f;
	tex[index++] = 0.0f;
	mPoints.emplace_back(mWidth, 0.0f, 0.0f);

	if (gradientAngle == 90 || gradientAngle == 270) {
		tex[index++] = 1.0f;
		tex[index++] = tex[1];
		mPoints.emplace_back(mWidth, mCenterY, 0.0f);
	}

	tex[index++] = 1.0f;
	tex[index++] = 1.0f;
	mPoints.emplace_back(mWidth, mHeight, 0.0f);
	bool b = createBufferObject((float*)mPoints.data(), numOfVertex *sizeof(Vec3), 
		numOfVertex,
		nullptr,0,
		tex,sizeof(tex));
	if (!b)
	{
		LOGD("error to load MeshFilledRect data\n");
	}
}

void MeshFilledRect::draw(int posloc, int texloc, int norloc, int colorloc, int tangentloc) {
	if (mbFilled) {
		drawTriangleFan(posloc, texloc, norloc, colorloc, tangentloc);
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
			assert(mCountOfVertex != 0);
			int componentOfPos = mPosByteSize / (sizeof(GLfloat) * mCountOfVertex);
			glVertexAttribPointer(posloc, componentOfPos, GL_FLOAT, GL_FALSE, 0,0);
		}
		else {

		}
		glBindVertexArray(0);
	}

	glBindVertexArray(mVAO);
	glLineWidth(mLineWidth);
	glDrawArrays(GL_LINE_STRIP, 1, mCountOfVertex-1);
	glBindVertexArray(0);
}