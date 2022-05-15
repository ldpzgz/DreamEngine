#pragma once

#include "Renderable.h"
#include <glm/vec3.hpp>           // vec3
enum class LightType {
	Directional,
	Point,
	SpotLight
};

constexpr int MaxNumberOfLights=15;

class Light :public Renderable {
public:
	explicit Light(LightType type);
	Light(LightType type,bool bCastShadow);
	Light() = default;

	bool isDirectionalLight() {
		return mType == LightType::Directional;
	}
	bool isPointLight() {
		return mType == LightType::Point;
	}
	bool isSpotLight() {
		return mType == LightType::SpotLight;
	}
	void setPosOrDir(const glm::vec3& pd) noexcept{
		mPosOrDir = pd;
	}

	const glm::vec3& getPosOrDir() const noexcept{
		return mPosOrDir;
	}

	glm::vec3& getPosOrDir() noexcept {
		return mPosOrDir;
	}

	void setLightColor(const glm::vec3& color) noexcept {
		mLightColor = color;
	}

	const glm::vec3& getLightColor() const noexcept {
		return mLightColor;
	}

	glm::vec3& getLightColor() noexcept {
		return mLightColor;
	}

	void setCastShadow(bool b) {
		mbCastShadow = b;
	}
	bool getCastShadow() {
		return mbCastShadow;
	}

	void draw(const glm::mat4* modelMat,
		const glm::mat4* texMat = nullptr,
		const glm::mat4* projViewMat = nullptr) override;
	
private:
	bool mbCastShadow{false};
	LightType mType{ LightType::Point };
	glm::vec3 mPosOrDir{-1.0f,-1.0f,0.0f};
	glm::vec3 mLightColor{1.0f,1.0f,1.0f};
};