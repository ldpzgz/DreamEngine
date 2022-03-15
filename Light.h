#ifndef _LIGHT_H_
#define _LIGHT_H_
#include "Renderable.h"
#include <glm/vec3.hpp>           // vec3
enum class LightType {
	Directional,
	Point,
	SpotLight
};

class Light :public Renderable {
public:
	explicit Light(LightType type);
	Light() = default;
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

	void draw(const glm::mat4* projMat,
		const glm::mat4* modelMat,
		const glm::mat4* texMat = nullptr,
		const std::vector<glm::vec3>* lightPos = nullptr,
		const std::vector<glm::vec3>* lightColor = nullptr,
		const glm::vec3* viewPos = nullptr) override;

private:
	LightType mType{ LightType::Point };
	glm::vec3 mPosOrDir{-1.0f,-1.0f,0.0f};
	glm::vec3 mLightColor{1.0f,1.0f,1.0f};
};
#endif