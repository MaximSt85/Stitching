#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>
#include <opencv2/core.hpp>
#include <opencv2/imgproc.hpp>
#include <opencv2/videoio.hpp>
#include <opencv2/highgui.hpp>
#include <unistd.h>
#include <ctime>

#include <tango.h>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <vector>

#include <thread>

using namespace curlpp::options;
using namespace cv;
using namespace std;

double start_position_linear_axe;
double end_position_linear_axe;
float conversionFactor;
double positionTriggerStart;
int nbTriggers;
double start_position_y_axe;
int nb_steps;

class Camera{
public:
	void switchStartFrameTriggerMode(int mode);
	int fixedRate = 3;
	int triggerMode = 1;
	int freerunMode = 0;
private:
	
};

void Camera::switchStartFrameTriggerMode(int mode){
	try {
		curlpp::Cleanup myCleanup;
		curlpp::Easy myRequest;
		if (mode == 0) {
			myRequest.setOpt<Url>("http://192.168.39.1:8082/?action=command&id=13&dest=0&group=42&value=0&plugin=0");
		}
		else if (mode == 1) {
			myRequest.setOpt<Url>("http://192.168.39.1:8082/?action=command&id=13&dest=0&group=42&value=1&plugin=0");
		}
		else if (mode == 3) {
			myRequest.setOpt<Url>("http://192.168.39.1:8082/?action=command&id=13&dest=0&group=42&value=3&plugin=0");
		}
		else {return;}
		myRequest.perform();
	}
	catch(curlpp::RuntimeError & e)	{
		cout << e.what() << endl;
	}
	catch(curlpp::LogicError & e) {
		cout << e.what() << endl;
	}
}


class workImage{
public:
	workImage(vector<Mat> imageArray, vector<double> positionsPiLC, int widthBild1, int heightBild1, float conversionFactor1, float downsamplingFactor1);
	int convertMuToPixels(double position);
	void pasteImgToScene(int positionY, int correction_y);
private:
	float conversionFactor;
	float downsamplingFactor;
	int widthBild;
	int heightBild;
	vector<double> positionsPiLC;
	vector<Mat> imageArray;
};

workImage::workImage(vector<Mat> imageArray1, vector<double> positionsPiLC1, int widthBild1, int heightBild1, float conversionFactor1, float downsamplingFactor1) {
	positionsPiLC = positionsPiLC1;
	imageArray = imageArray1;
	widthBild = widthBild1;
	heightBild = heightBild1;
	conversionFactor = conversionFactor1;
	downsamplingFactor = downsamplingFactor1;
	//cout << "in constructor, positionsPiLC size: " << positionsPiLC.size() << endl;
	//cout << "in constructor, imageArray size: " << imageArray.size() << endl;
}

int workImage::convertMuToPixels(double position) {
	return static_cast<int>(position*conversionFactor);
}

void workImage::pasteImgToScene(int positionY, int correction_y) {
	//if (positionY > 40) {positionY -= 40;}
	//cout << "positionsPiLC size: " << positionsPiLC.size() << endl;
	Mat scene = imread("scene.jpg");
	//cout << "have read scene" << endl;
	//int arraySize = positionsPiLC.size();
	int arraySize = imageArray.size();
	for (int i=0;i<arraySize;i++) {
		//cout << "in loop" << i << endl;
		Mat src, dst;
		src = imageArray[i];
		int centerImgInScene = convertMuToPixels(positionsPiLC[i] + 2000);
		Point leftCornerInScene;
		leftCornerInScene.x = centerImgInScene - static_cast<int>(widthBild / 2) - 42*correction_y;
		leftCornerInScene.y = positionY + 1 + 55*i;
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

class pilc{
public:
	pilc(string path_to_device);

	void set_arm(int on_off);
	void set_nbTriggers(int nbTriggers_to_set);
	void set_positionTriggerStart(double positionTriggerStart_to_set);
	void set_positionTriggerStepSize(double positionTriggerStepSize_to_set);
	vector<double> get_position1Data();
	
private:
	Tango::DeviceProxy *pilc_device;	
	
	Tango::DeviceAttribute nbTriggers_attribute;
	Tango::DeviceAttribute positionTriggerStart_attribute;
	Tango::DeviceAttribute arm_attribute;
	Tango::DeviceAttribute timeTriggerStepSize_attribute;
	Tango::DeviceAttribute positionTriggerStepSize_attribute;
	Tango::DeviceAttribute position1Data_attribute;

	double positionTriggerStart;
	long arm;
	int nbTriggers;
	double timeTriggerStepSize;
	double positionTriggerStepSize;
	double calibrateEncoder1;
	vector<double> position1Data;

	string nbTriggers_name = "NbTriggers";
	string positionTriggerStart_name = "PositionTriggerStart";
	string arm_name = "Arm";
	string timeTriggerStepSize_name = "TimeTriggerStepSize";
	string positionTriggerStepSize_name = "PositionTriggerStepSize";
	string calibrateEncoder1_name = "CalibrateEncoder1";
	string position1Data_name = "Position1Data";
};

pilc::pilc(string path_to_device) {
	try {
		pilc_device = new Tango::DeviceProxy(path_to_device);
		arm_attribute = pilc_device->read_attribute(arm_name);
		nbTriggers_attribute = pilc_device->read_attribute(nbTriggers_name);
		positionTriggerStart_attribute = pilc_device->read_attribute(positionTriggerStart_name);
		positionTriggerStepSize_attribute = pilc_device->read_attribute(positionTriggerStepSize_name);
	}
	catch (Tango::DevFailed &e) {
		Tango::Except::print_exception(e);
    	}
}

void pilc::set_arm(int on_off) {
	if (on_off == 1) {
		try {
			//arm_attribute = pilc_device->read_attribute(arm_name);
			//arm_attribute >> arm;
			Tango::DevVarLongArray *in = new Tango::DevVarLongArray();
			in->length(2);
			(*in)[0] = 1;
			(*in)[1] = 1;
			arm_attribute << in;
			pilc_device->write_attribute(arm_attribute);
  		}
		catch (Tango::DevFailed &e) {
			Tango::Except::print_exception(e);
	    	}
	}
	else {
		try {
			//arm_attribute = pilc_device->read_attribute(arm_name);
			//arm_attribute >> arm;
			Tango::DevVarLongArray *in = new Tango::DevVarLongArray();
			in->length(2);
			(*in)[0] = 1;
			(*in)[1] = 0;
			arm_attribute << in;
			pilc_device->write_attribute(arm_attribute);
  		}
		catch (Tango::DevFailed &e) {
			Tango::Except::print_exception(e);
	    	}
	}
}

void pilc::set_nbTriggers(int nbTriggers_to_set) {
	try {
		//nbTriggers_attribute = pilc_device->read_attribute(nbTriggers_name);
		//nbTriggers_attribute >> nbTriggers;
		nbTriggers = nbTriggers_to_set;
		nbTriggers_attribute << nbTriggers;
		pilc_device->write_attribute(nbTriggers_attribute);
	}
	catch (Tango::DevFailed &e) {
		Tango::Except::print_exception(e);
	}
}

void pilc::set_positionTriggerStart(double positionTriggerStart_to_set) {
	try {
		//positionTriggerStart_attribute = pilc_device->read_attribute(positionTriggerStart_name);
		//positionTriggerStart_attribute >> positionTriggerStart;
		positionTriggerStart = positionTriggerStart_to_set;
		positionTriggerStart_attribute << positionTriggerStart;
		pilc_device->write_attribute(positionTriggerStart_attribute);
	}
	catch (Tango::DevFailed &e) {
		Tango::Except::print_exception(e);
	}
}

void pilc::set_positionTriggerStepSize(double positionTriggerStepSize_to_set) {
	try {
		//positionTriggerStepSize_attribute = pilc_device->read_attribute(positionTriggerStepSize_name);
		//positionTriggerStepSize_attribute >> positionTriggerStepSize;
		positionTriggerStepSize = positionTriggerStepSize_to_set;
		positionTriggerStepSize_attribute << positionTriggerStepSize;
		pilc_device->write_attribute(positionTriggerStepSize_attribute);
	}
	catch (Tango::DevFailed &e) {
		Tango::Except::print_exception(e);
	}
}

vector<double> pilc::get_position1Data() {
	try {
		position1Data_attribute = pilc_device->read_attribute(position1Data_name);
		position1Data_attribute >> position1Data;
		return position1Data;
	}
	catch (Tango::DevFailed &e) {
		Tango::Except::print_exception(e);
	}
}

class linear_axis{
public:
	linear_axis(string path_to_device);
	void set_position(double position_to_set);
	int get_state();
private:
	Tango::DeviceProxy *linear_axis_device;	
	Tango::DeviceAttribute position_attribute;
	double position;
	string position_name = "Position";
	int state;
};

linear_axis::linear_axis(string path_to_device) {
	linear_axis_device = new Tango::DeviceProxy(path_to_device);
	position_attribute = linear_axis_device->read_attribute(position_name);
}

void linear_axis::set_position(double position_to_set) {
	try {
		//linear_axis_device->read_attribute(position_name);
		//cout << "hello" << endl;
		//position_attribute = linear_axis_device->read_attribute(position_name);
		Tango::DevVarDoubleArray *in = new Tango::DevVarDoubleArray();
		in->length(1);
		(*in)[0] = position_to_set;
		position_attribute << in;
		//cout << "position_attribute to set: " << position_attribute;
		linear_axis_device->write_attribute(position_attribute);
  	}
	catch (Tango::DevFailed &e) {
		Tango::Except::print_exception(e);
	}
	catch (CORBA::BAD_PARAM &e) {
		Tango::Except::print_exception(e);
	}
}

int linear_axis::get_state() {
	try {
		state = linear_axis_device->state();
		return state;
  	}
	catch (Tango::DevFailed &e) {
		Tango::Except::print_exception(e);
	}
}

class y_axis{
public:
	y_axis(string path_to_device);
	
	void set_position(double position_to_set);
	int get_state();
private:
	Tango::DeviceProxy *y_axis_device;	
	
	Tango::DeviceAttribute position_attribute;
	double position;
	string position_name = "Position";
	int state;
};

y_axis::y_axis(string path_to_device) {
	y_axis_device = new Tango::DeviceProxy(path_to_device);
	position_attribute = y_axis_device->read_attribute(position_name);
}

void y_axis::set_position(double position_to_set) {
	try {
		Tango::DevVarDoubleArray *in = new Tango::DevVarDoubleArray();
		in->length(1);
		(*in)[0] = position_to_set;
		position_attribute << in;
		y_axis_device->write_attribute(position_attribute);
  	}
	catch (Tango::DevFailed &e) {
		Tango::Except::print_exception(e);
	}
	catch (CORBA::BAD_PARAM &e) {
		Tango::Except::print_exception(e);
	}
}

int y_axis::get_state() {
	try {
		state = y_axis_device->state();
		return state;
  	}
	catch (Tango::DevFailed &e) {
		Tango::Except::print_exception(e);
	}
}

void takeImages(vector<Mat>& imageArray, VideoCapture& cap, int nbTriggers) {
	int counter = 0;
	int counter1 = 0;
	nbTriggers -= 1;
	while(true) {
		auto begin = chrono::high_resolution_clock::now();
		Mat img;
		cap >> img;
		auto end = chrono::high_resolution_clock::now();
		float elapsed_secs = chrono::duration_cast<std::chrono::nanoseconds>(end-begin).count()/1000000000;
		cout << "elapsed_secs: " << elapsed_secs << endl;
		counter1++;
		//cout << "counter1: " << counter1 << endl;
		if (elapsed_secs > 0.030) {
			imageArray.push_back(img);
			//imwrite("1.jpg", img);
			break;
		}
	}
	//namedWindow("Video", WINDOW_AUTOSIZE);
	counter++;
	cout << "taking pictures when triggering..." << endl;
	for (;;) {
		Mat img1;
		cap >> img1;
		//imshow("Video", img1);
		//int key = waitKey(1);
		//cv::Mat dst;
		//cv::flip(img1, dst, 0);
		imageArray.push_back(img1);
		counter++;
		//cout << "counter is: " << counter << endl;
		//ostringstream ss;
		//ss << counter;
		//string filename;
		//filename = ss.str() + ".jpg";
		//imwrite(filename, img1);
		if (counter == nbTriggers) {break;}
	}
}

void printUsage(char** argv)
{
	cout << "Usage:\n" << argv[0] <<" [Flags] [Values] \n\n"
	"Flags:\n"
	"  --start_position_linear_axe\n"
	"      start position of linear axe\n"
	"  --end_position_linear_axe\n"
	"      end position of linear axe\n"
	"  --conversionFactor\n"
	"      conversion factor pixel to mu [pexels / mu]\n"
	"  --positionTriggerStart\n"
	"      position for PiLC to start triggering\n"
	"  --nbTriggers\n"
	"      number of triggers for PiLC\n"
	"  --start_position_y_axe\n"
	"      start position of y axe (y axe actuator)\n"
	"  --nb_steps\n"
	"      number of steps to scan in y axe direction\n"
	"Example usage :\n" << argv[0] << " --start_position_linear_axe 0 --end_position_linear_axe 36000 --conversionFactor 1.81 "
	"--positionTriggerStart 1000 --nbTriggers 48 --start_position_y_axe 0 --nb_steps 20\n";
}

bool legal_int(char *str)
{
	bool isTherePoint = false;
	while (*str != '\0') {
		if (!isdigit(*str)) {
			if (*str == '.' && isTherePoint == true) {return false;}
			if (*str == '.') {isTherePoint = true;}
			if (*str != '.') {return false;}
		}
		str++;
	}
	return true;
}

int parseCmdArgs(int argc, char** argv)
{
	if (argc == 1) {
		cout << "Default settings" << endl;
		start_position_linear_axe = 0;
		end_position_linear_axe = 36000;
		conversionFactor = 1.81;
		positionTriggerStart = 1000;
		nbTriggers = 47;
		start_position_y_axe = 0;
		nb_steps = 20;
		return 0;
	}
	if (argc != 1 & argc != 15) {
		cout << "Missing parameter" << endl;
		printUsage(argv);
		return -1;
	}
	for (int i = 1; i < argc; ++i) {
		if (string(argv[i]) == "--help" || string(argv[i]) == "/?") {
			printUsage(argv);
			return -1;
		}
		else if (string(argv[i]) == "--start_position_linear_axe") {
			if (legal_int(argv[i + 1]) == false) {
				cout << "Bad --start_position_linear_axe flag value\n";
				return -1;
			}
			start_position_linear_axe = atof(argv[i + 1]);
			if (start_position_linear_axe < 0) {
				cout << "Bad --start_position_linear_axe flag value, must be more or equal 0\n";
				return -1;
			}
			i++;
		}
		else if (string(argv[i]) == "--end_position_linear_axe") {
			if (legal_int(argv[i + 1]) == false) {
				cout << "Bad --end_position_linear_axe flag value\n";
				return -1;
			}
			end_position_linear_axe = atof(argv[i + 1]);
			if (end_position_linear_axe <= 0) {
				cout << "Bad --end_position_linear_axe flag value, must be more than 0\n";
				return -1;
			}
			i++;
		}
		else if (string(argv[i]) == "--conversionFactor") {
			if (legal_int(argv[i + 1]) == false) {
				cout << "Bad --conversionFactor flag value\n";
				return -1;
			}
			conversionFactor = atof(argv[i + 1]);
			if (conversionFactor < 0) {
				cout << "Bad --conversionFactor flag value, must be more or equal 0\n";
				return -1;
			}
			i++;
		}
		else if (string(argv[i]) == "--positionTriggerStart") {
			if (legal_int(argv[i + 1]) == false) {
				cout << "Bad --positionTriggerStart flag value\n";
				return -1;
			}
			positionTriggerStart = atof(argv[i + 1]);
			if (positionTriggerStart < 0) {
				cout << "Bad --positionTriggerStart flag value, must be more or equal 0\n";
				return -1;
			}
			i++;
		}
		else if (string(argv[i]) == "--nbTriggers") {
			if (legal_int(argv[i + 1]) == false) {
				cout << "Bad --nbTriggers flag value\n";
				return -1;
			}
			nbTriggers = atof(argv[i + 1]);
			if (nbTriggers <= 0) {
				cout << "Bad --nbTriggers flag value, must be more than 0\n";
				return -1;
			}
			i++;
		}
		else if (string(argv[i]) == "--start_position_y_axe") {
			if (legal_int(argv[i + 1]) == false) {
				cout << "Bad --start_position_y_axe flag value\n";
				return -1;
			}
			start_position_y_axe = atof(argv[i + 1]);
			i++;
		}
		else if (string(argv[i]) == "--nb_steps") {
			if (legal_int(argv[i + 1]) == false) {
				cout << "Bad --nb_steps flag value\n";
				return -1;
			}
			nb_steps = atof(argv[i + 1]);
			if (nb_steps < 0) {
				cout << "Bad --nb_steps flag value, must be more or equal 0\n";
				return -1;
			}
			i++;
		}
		else {
			cout << "Bad flag " << argv[i] << endl;
			printUsage(argv);
			return -1;
		}
	}
	return 0;
}

int main(int argc, char** argv ) {

	int retval = parseCmdArgs(argc, argv);
    	if (retval) return -1;

	cout << "start_position_linear_axe " << start_position_linear_axe << endl;
	cout << "end_position_linear_axe " << end_position_linear_axe << endl;
	cout << "conversionFactor " << conversionFactor << endl;
	cout << "positionTriggerStart " << positionTriggerStart << endl;
	cout << "nbTriggers " << nbTriggers << endl;
	cout << "start_position_y_axe " << start_position_y_axe << endl;
	cout << "nb_steps " << nb_steps << endl;

	if (end_position_linear_axe < start_position_linear_axe) {
		cout << "end position of linear axe must be more than srart position\n";
		return -1;
	}
	if (positionTriggerStart > end_position_linear_axe || positionTriggerStart < start_position_linear_axe) {
		cout << "position trigger start must be less than end position of linear axe and must be more than srart position\n";
		return -1;
	}

	int widthBild = 1360;
	int heightBild = 1024;

	if (nbTriggers > ((end_position_linear_axe - positionTriggerStart) / (widthBild / conversionFactor) + 1)) {
		cout << "to many triggers\n";
		return -1;
	}

	//return 0;

	//float conversionFactor = 1.81;
	float downsamplingFactor = 1360 / 640;
    	int widthScene = (end_position_linear_axe - start_position_linear_axe + 2000) * conversionFactor / downsamplingFactor;
	int heightScene = (heightBild * nb_steps + 2000 * conversionFactor) / downsamplingFactor;
	Mat scene = Mat(heightScene, widthScene, CV_8UC3, Scalar(176, 226, 255));
	imwrite("scene.jpg", scene);
	
	vector<double> position1Data;
	vector<Mat> imageArray;
	
	//int nbTriggers = 48;
	//double start_position_linear_axe = 0;
	//double end_position_linear_axe = 36000;
	//double start_position_y_axe = 0;
	//double positionTriggerStart = 1000;

	//int nb_steps = 20;

	pilc myPilc("haspp11mexp5:10000/p11/pilc/pilc-trigger-generator");
	linear_axis myLinear_axis("haspp11mexp5:10000/p11/motor/galil.02");
	y_axis myY_axis("haspp11mexp5:10000/p11/motor/galil.05");

	myLinear_axis.set_position(start_position_linear_axe);
	myY_axis.set_position(start_position_y_axe);
	while (myLinear_axis.get_state() == Tango::MOVING) {
		sleep(0.1);
	}
	while (myY_axis.get_state() == Tango::MOVING) {
		sleep(0.1);
	}
	myPilc.set_nbTriggers(nbTriggers);
	myPilc.set_positionTriggerStepSize(widthBild / conversionFactor);
	myPilc.set_positionTriggerStart(positionTriggerStart);
	cout << "########## all parameters setted ##########" << endl;

	for (int k=0;k<nb_steps; k++) {
		cout << "########## step: " << k << "##########"<< endl;
		myY_axis.set_position(-1024 * k / 1.81);
		myLinear_axis.set_position(start_position_linear_axe);
		while (myY_axis.get_state() == Tango::MOVING) {
			sleep(0.1);
		}
		while (myLinear_axis.get_state() == Tango::MOVING) {
			sleep(0.1);
		}
		myPilc.set_arm(1);
		sleep(0.3);
		cout << "########## armed pilc and setted motors to their start positions ##########" << endl;

		Camera myCamera;
		myCamera.switchStartFrameTriggerMode(myCamera.freerunMode);
		//myCamera.switchStartFrameTriggerMode(myCamera.fixedRate);
		VideoCapture cap("http://192.168.39.1:8082/?action=stream/frame.mjpg");
		std::thread takeImagesThread (takeImages, ref(imageArray), ref(cap), nbTriggers);
		myCamera.switchStartFrameTriggerMode(myCamera.triggerMode);
		sleep(2);
		cout << "########## captured video ##########" << endl;

		myLinear_axis.set_position(end_position_linear_axe);
		takeImagesThread.join();
		position1Data = myPilc.get_position1Data();
		//cout << "size of position1Data: " << position1Data.size() << endl;
		//cout << "size of imageArray: " << imageArray.size() << endl;
		cout << "########## finished scan ##########" << endl;

		workImage myImage(imageArray, position1Data, widthBild, heightBild, conversionFactor, downsamplingFactor);
		myImage.pasteImgToScene(1024 * k, k);
		cout << "########## wrote scene ##########" << endl;
		
		imageArray.clear();
		position1Data.clear();
	}

	myLinear_axis.set_position(start_position_linear_axe);
	myY_axis.set_position(start_position_y_axe);
	while (myLinear_axis.get_state() == Tango::MOVING) {
		sleep(0.1);
	}
	while (myY_axis.get_state() == Tango::MOVING) {
		sleep(0.1);
	}
	
	return 0;
}






/*myCamera.switchStartFrameTriggerMode(myCamera.freerunMode);
	//myCamera.switchStartFrameTriggerMode(myCamera.fixedRate);
	VideoCapture cap1("http://192.168.39.1:8082/?action=stream/frame.mjpg");
	std::thread takeImagesThread1 (takeImages, ref(imageArray), ref(cap1), nbTriggers);
	myCamera.switchStartFrameTriggerMode(myCamera.triggerMode);
	sleep(1);
	
	myY_axis.set_position(-1024 / 1.81);
	while (myY_axis.get_state() == Tango::MOVING) {
		sleep(0.1);
	}
	cout << "moved y axe to 565" << endl;
	sleep(2);

	myPilc.set_arm(1);
	cout << "armed pilc" << endl;
	sleep(2);
	
	myLinear_axis.set_position(end_position_linear_axe);

	takeImagesThread1.join();
	position1Data = myPilc.get_position1Data();
	cout << "size of position1Data: " << position1Data.size() << endl;
	cout << "size of imageArray: " << imageArray.size() << endl;
	workImage myImage1(imageArray, position1Data);
	myImage1.pasteImgToScene(1024);*/





/*timeTriggerStepSize_attribute = pilc_device->read_attribute(timeTriggerStepSize_name);
	timeTriggerStepSize_attribute >> timeTriggerStepSize;
	timeTriggerStepSize = 0.077;
	timeTriggerStepSize_attribute << timeTriggerStepSize;
	pilc_device->write_attribute(timeTriggerStepSize_attribute);*/



/*Tango::DeviceData calibrateEncoder1_data;
	try {
		Tango::DevDouble position_to_calibrate;
		position_to_calibrate = position * conversion_factor;
		calibrateEncoder1_data << position_to_calibrate;
    		pilc_device->command_inout(calibrateEncoder1_name, calibrateEncoder1_data);
  	}
	catch (Tango::DevFailed &e) {
		Tango::Except::print_exception(e);
    	}*/



//cap.set(CV_CAP_PROP_FRAME_COUNT,0);
	//cap.set(CV_CAP_PROP_BUFFERSIZE,3);
	/*while (counter1 < 50) {
		cout << "cap.get(CV_CAP_PROP_BUFFERSIZE): " << cap.get(CV_CAP_PROP_BUFFERSIZE) << endl;
		//cout << "cap.get(CV_CAP_PROP_FRAME_COUNT): " << cap.get(CV_CAP_PROP_FRAME_COUNT) << endl;
		//cout << "cap.get(CV_CAP_PROP_POS_AVI_RATIO): " << cap.get(CV_CAP_PROP_POS_AVI_RATIO) << endl;
		//cout << "cap.get(CV_CAP_PROP_POS_MSEC): " << cap.get(CV_CAP_PROP_POS_MSEC) << endl;
		//cout << "cap.get(CV_CAP_PROP_POS_FRAMES): " << cap.get(CV_CAP_PROP_POS_FRAMES) << endl;
		//cout << "cap.get(CV_CAP_PROP_MODE): " << cap.get(CV_CAP_PROP_MODE) << endl;
		//cout << "cap.isOpened(): " << cap.isOpened() << endl;
		Mat img1;
		cap >> img1;
		counter1++;
		cout << "counter1 is: " << counter1 << endl;
	}*/


//namedWindow("Video", WINDOW_AUTOSIZE);
//cout << "in the loop" << endl;
//clock_t begin = clock();
//imshow("Video", img);
			//int key = waitKey(1);
			//cout << "key is: " << key << endl;
			//if (key == 27) {break;}
			//sleep(1);
			//clock_t end = clock();
  			//double elapsed_secs = double(end - begin) / CLOCKS_PER_SEC;
			//cout << "elapsed time is: " << elapsed_secs << endl;
			//cout << "Width : " << img.size().width << endl;
			//cout << "Height: " << img.size().height << endl;
			//cout << "Channels: " << img.channels() << endl;
			//cout << "Pixel " << img[100,100] << endl;
			//break;


/*if (counter == 100) {
				counter = 0;
				ostringstream ss;
				num++;
				ss << num;
				filename = ss.str() + ".jpg";
				imwrite(filename, img);
			}*/

			/*counter++;
			ostringstream ss;
			ss << counter;
			filename = ss.str() + ".jpg";
			imwrite(filename, img);*/



/*if (pilc_device->state() == Tango::ON) {
		cout << "hello" << endl;
	}
	cout << "pilc state: " << pilc_device->state() << endl;
	cout << "pilc status: " << pilc_device->status() << endl;*/







/*try {
		// That's all that is needed to do cleanup of used resources (RAII style).
		curlpp::Cleanup myCleanup;
		// Our request to be sent.
		curlpp::Easy myRequest;
		// Set the URL.
		//myRequest.setOpt<Url>("http://google.com");
		myRequest.setOpt<Url>("http://192.168.39.1:8082/?action=command&id=13&dest=0&group=42&value=0&plugin=0");
		
		// Send request and get a result.
		// By default the result goes to standard output.
		myRequest.perform();
	}
	catch(curlpp::RuntimeError & e)	{
		std::cout << e.what() << std::endl;
	}
	catch(curlpp::LogicError & e) {
		std::cout << e.what() << std::endl;
	}

	VideoCapture cap("http://192.168.39.1:8082/?action=stream/frame.mjpg");

	try {
		// That's all that is needed to do cleanup of used resources (RAII style).
		curlpp::Cleanup myCleanup;
		// Our request to be sent.
		curlpp::Easy myRequest;
		// Set the URL.
		//myRequest.setOpt<Url>("http://google.com");
		myRequest.setOpt<Url>("http://192.168.39.1:8082/?action=command&id=13&dest=0&group=42&value=1&plugin=0");
		
		// Send request and get a result.
		// By default the result goes to standard output.
		myRequest.perform();
		std::ostringstream os;
		os << myRequest;
		std::cout << os.str() << std::endl;
	}
	catch(curlpp::RuntimeError & e)	{
		std::cout << e.what() << std::endl;
	}
	catch(curlpp::LogicError & e) {
		std::cout << e.what() << std::endl;
	}*/






