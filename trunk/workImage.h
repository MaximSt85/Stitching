#ifndef WORKIMAGE_H_
#define WORKIMAGE_H_

#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>

#include <vector>

using namespace cv;

class workImage {
public:
	workImage(vector<Mat> imageArray, vector<double> positionsPiLC, int widthBild1, int heightBild1, float conversionFactor1,
			float downsamplingFactor1, double tuning_parameter_linear_axe1, double tuning_parameter_y_axe1);
	int convertMuToPixels(double position);
	void pasteImgToScene(int positionY, double correction_y);
private:
	float conversionFactor;
	float downsamplingFactor;
	int widthBild;
	int heightBild;
	vector<double> positionsPiLC;
	vector<Mat> imageArray;
	double tuning_parameter_linear_axe;
	double tuning_parameter_y_axe;
};
#endif
