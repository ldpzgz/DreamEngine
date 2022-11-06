#include "BackgroundStyle.h"
#include "core/MeshFilledRect.h"
#include "Shape.h"
#include "core/Texture.h"
void BackgroundStyle::setMyStyle() {
	if (mpShape) {
		mpShape->setTexture(mpTex);
		mpShape->setGradientStartColor(mStartColor);
		mpShape->setGradientCenterColor(mCenterColor);
		mpShape->setGradientEndColor(mEndColor);
		mpShape->setSolidColor(mSolidColor);
		mpShape->setBorderColor(mBorderColor);
	}
}

void BackgroundStyle::setTexture(const std::shared_ptr<Texture>& pTex) {
	mpTex = pTex;
	if (mpShape) {
		mpShape->setTexture(mpTex);
	}
}

void BackgroundStyle::setStartColor(const Color& c) {
	mStartColor = c;
	if (mpShape) {
		mpShape->setGradientStartColor(c);
	}
}

void BackgroundStyle::setCenterColor(const Color& c) {
	mCenterColor = c;
	if (mpShape) {
		mpShape->setGradientCenterColor(c);
	}
}

void BackgroundStyle::setEndColor(const Color& c) {
	mEndColor = c;
	if (mpShape) {
		mpShape->setGradientEndColor(c);
	}
}

void BackgroundStyle::setBorderColor(const Color& c) {
	mBorderColor = c;
	if (mpShape) {
		mpShape->setBorderColor(c);
	}
}

void BackgroundStyle::setSolidColor(const Color& c) {
	mSolidColor = c;
	if (mpShape) {
		mpShape->setSolidColor(c);
	}
}

MeshFilledRect* BackgroundStyle::getBkMesh() {
	if (mpBkMesh) {
		return mpBkMesh.get();
	}
	return nullptr;
}

MeshFilledRect* BackgroundStyle::getBkBorderMesh() {
	if (mpBkBorderMesh) {
		return mpBkBorderMesh.get();
	}
	return nullptr;
}
void BackgroundStyle::setBkMesh(std::shared_ptr<MeshFilledRect>& pMesh) {
	mpBkMesh = pMesh;
}
void BackgroundStyle::setBkBorderMesh(std::shared_ptr<MeshFilledRect>& pMesh) {
	mpBkBorderMesh = pMesh;
}

