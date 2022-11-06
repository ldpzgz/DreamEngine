#pragma once

class Vbo {
public:
	Vbo() = default;
	~Vbo();
	bool initVbo(void* pdata,int sizeInByte);
	bool updateVbo(int offset,void* pdata,int sizeInByte);

	void bindArray(bool b);
	void bindElement(bool b);
private:
	unsigned int mId{0};
	int mTotalSize{ 0 };
};

