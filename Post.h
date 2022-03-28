#pragma once
#include <memory>
class Mesh;
class Texture;
class Post {
public:
	Post();
	virtual ~Post();
	virtual void initPost(int width, int height, std::shared_ptr<Mesh>& pMesh);
	virtual void process(std::shared_ptr<Texture>& pInput, std::shared_ptr<Texture>& pOutput)=0;
protected:
	std::shared_ptr<Mesh> mpMesh;
	std::shared_ptr<Texture> mpInputTex;
	std::shared_ptr<Texture> mpOutputTex;
};