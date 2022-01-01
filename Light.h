#ifndef _LIGHT_H_
#define _LIGHT_H_
#include "Attachable.h"
#include "Rect.h"
enum class LightType {
	Directional,
	Point,
	SpotLight
};

class Light :public Attachable {
public:
	explicit Light(LightType type);
	Light() = default;
	void setPosOrDir(const Vec3& pd) {
		mPosOrDir = pd;
	}

	const Vec3& getPosOrDir() {
		return mPosOrDir;
	}

	void setLightColor(const Vec3& color) {
		mLightColor = color;
	}

	const Vec3& getLightColor() {
		return mLightColor;
	}
private:
	LightType mType{ LightType::Point };
	Vec3 mPosOrDir{-1.0f,-1.0f,0.0f};
	Vec3 mLightColor{1.0f,1.0f,1.0f};
};
#endif