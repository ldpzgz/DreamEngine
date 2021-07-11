#include "TextureUtils.h"
#include "Texture.h"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/imgproc/types_c.h"
using namespace std;
std::shared_ptr<Texture> loadImageFromFile(const std::string& path) {
	shared_ptr<Texture> retTex;
	cv::Mat inImage;
	int internalFormat = GL_RGB;
	inImage = cv::imread(path);

	if (!inImage.empty()) {
		int width = inImage.cols;
		int height = inImage.rows;
		int channels = inImage.channels();
		cout << "image channels " << channels << " image width " << width << " height  " << height << endl;

		if (channels == 3) {
			cv::cvtColor(inImage, inImage, CV_RGB2BGR);
			internalFormat = GL_RGB;
		}else if (channels == 4) {
			cv::cvtColor(inImage, inImage, CV_RGBA2BGRA);
			internalFormat = GL_RGBA;
		}else if (channels == 1) {
			internalFormat = GL_LUMINANCE;
		}else {
			return retTex;
		}

		cv::flip(inImage, inImage, 0);
		unsigned char* pdata = inImage.data;
		
		
		retTex = make_shared<Texture>();
		retTex->load(width, height, pdata, internalFormat);
	}
	return retTex;
}