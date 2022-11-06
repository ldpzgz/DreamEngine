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
	void setNextPost(const std::shared_ptr<Post>& pNext) {
		mpNext = pNext;
	}

	inline bool processWrap(std::shared_ptr<Texture>& pInput, std::shared_ptr<Texture>& pOutput) {
		if (mbOpen) {
			process(pInput, pOutput);
			return true;
		}
		return false;
	}
	static std::shared_ptr<Texture> doPost(std::shared_ptr<Post>& pPost,std::shared_ptr<Texture> inTex, std::shared_ptr<Texture> outTex);
protected:
	bool mbOpen{ true };
	std::shared_ptr<Mesh> mpMesh;
	std::shared_ptr<Texture> mpInputTex;
	std::shared_ptr<Texture> mpOutputTex;
	std::shared_ptr<Post> mpNext;
};