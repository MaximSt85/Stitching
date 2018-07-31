#include "workImage.h"

using namespace cv;


workImage::workImage(vector<Mat> imageArray1, vector<double> positionsPiLC1, int widthBild1, int heightBild1, float conversionFactor1,
		float downsamplingFactor1, double tuning_parameter_linear_axe1, double tuning_parameter_y_axe1) {
	positionsPiLC = positionsPiLC1;
	imageArray = imageArray1;
	widthBild = widthBild1;
	heightBild = heightBild1;
	conversionFactor = conversionFactor1;
	downsamplingFactor = downsamplingFactor1;
	tuning_parameter_linear_axe = tuning_parameter_linear_axe1;
	tuning_parameter_y_axe = tuning_parameter_y_axe1;
	//cout << "in constructor, positionsPiLC size: " << positionsPiLC.size() << endl;
	//cout << "in constructor, imageArray size: " << imageArray.size() << endl;
}

int workImage::convertMuToPixels(double position) {
	return static_cast<int>(position*conversionFactor);
}

void workImage::pasteImgToScene(int positionY, double correction_y) {
	//if (positionY > 40) {positionY -= 40;}
	Mat scene = imread("scene.jpg");
	int arraySize = imageArray.size();
	for (int i=0;i<arraySize;i++) {
		Mat src, dst;
		src = imageArray[i];
		int centerImgInScene = convertMuToPixels(positionsPiLC[i] + 2000);
		Point leftCornerInScene;
		leftCornerInScene.x = centerImgInScene - static_cast<int>(widthBild / 2) - tuning_parameter_linear_axe * correction_y;
		leftCornerInScene.y = convertMuToPixels(positionY) + 1 + tuning_parameter_y_axe * i;
		if (downsamplingFactor > 0) {
			Size size(widthBild / downsamplingFactor, heightBild / downsamplingFactor);
			resize(src, dst, size);
			//ostringstream ss;
			//ss << i;
			//string filename;
			//filename = ss.str() + ".jpg";
			//imwrite(filename, dst);
			leftCornerInScene.x = static_cast<int>(leftCornerInScene.x / downsamplingFactor);
			leftCornerInScene.y = static_cast<int>(leftCornerInScene.y / downsamplingFactor);
		}
		else {
			dst = imageArray[i];
		}
		dst.copyTo(scene(Rect(leftCornerInScene.x, leftCornerInScene.y, dst.cols, dst.rows)));
	}
	cout << "########## writing scene... ##########" << endl;
	imwrite("scene.jpg", scene);
}
