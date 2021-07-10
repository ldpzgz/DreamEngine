#include "TextureUtils.h"
#include "Texture.h"
using namespace std;
std::shared_ptr<Texture> loadImageFromFile(const std::string& path) {
	shared_ptr<Texture> retTex;
	cv::Mat inImage;
	inImage = cv::imread(path);
	if (!inImage.empty()) {
		int width = inImage.cols;
		int height = inImage.rows;
		unsigned char* pdata = inImage.data;
		int internalFormat = GL_RGB;
		cout << "image channels " << inImage.channels() << endl;
		cout << "image width " << width <<" height  "<< height << endl;
		int channels = inImage.channels();
		if (channels == 3) {
			internalFormat = GL_RGB;
		}
		else if (channels == 1) {
			internalFormat = GL_LUMINANCE;
		}
		else {
			return retTex;
		}
		retTex = make_shared<Texture>();
		retTex->load(width, height, pdata, internalFormat);
	}
	return retTex;
}