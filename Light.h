#ifndef _LIGHT_H_
#define _LIGHT_H_
#include "Attachable.h"

enum class LightType {
	Directional,
	Point,
	SpotLight
};
template<class T>
class Vector3;

class Light :public Attachable {
public:
	explicit Light(LightType type);
	Light() = default;
	void setPosOrDir(const Vector3<float>& pd) noexcept{
		mPosOrDir = pd;
	}

	const Vector3<float>& getPosOrDir() const noexcept{
		return mPosOrDir;
	}

	Vector3<float>& getPosOrDir() noexcept {
		return mPosOrDir;
	}

	void setLightColor(const Vector3<float>& color) noexcept {
		mLightColor = color;
	}

	const Vector3<float>& getLightColor() const noexcept {
		return mLightColor;
	}

	Vector3<float>& getLightColor() noexcept {
		return mLightColor;
	}
private:
	LightType mType{ LightType::Point };
	Vector3<float> mPosOrDir{-1.0f,-1.0f,0.0f};
	Vector3<float> mLightColor{1.0f,1.0f,1.0f};
};
#endif