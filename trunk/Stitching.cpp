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

/*#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>*/

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/ini_parser.hpp>

//#include <boost/thread.hpp>

#include <vector>

#include <thread>

#include "camera.cpp"
#include "workImage.cpp"
#include "pilc.cpp"
//#include "linear_axis.cpp"
//#include "y_axis.cpp"
#include "captureImage.cpp"

//using namespace curlpp::options;
using namespace cv;
using namespace std;

double start_position_linear_axe;
double end_position_linear_axe;
double conversionFactor;
double start_position_y_axe;
double end_position_y_axe;
double current_position_y_axe;
double positionTriggerStart;


void printUsage(char** argv)
{
	cout << "Usage:\n" << argv[0] <<" [Flags] [Values] \n\n"
	"Flags:\n"
	"  --start_position_linear_axe\n"
	"      start position of linear axe\n"
	"  --end_position_linear_axe\n"
	"      end position of linear axe\n"
	"  --start_position_y_axe\n"
	"      start position of y axe (y axe actuator)\n"
	"  --end_position_y_axe\n"
	"      end position of y axe (y axe actuator)\n"
	"  --current_position_y_axe\n"
	"      current position of y axe (y axe actuator)\n"
	"Example usage :\n" << argv[0] << " --start_position_linear_axe 0 --end_position_linear_axe 36000 --start_position_y_axe 0 --end_position_y_axe 9000 --current_position_y_axe 5000\n";
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
	/*if (argc == 1) {
		cout << "Default settings" << endl;
		start_position_linear_axe = 0;
		end_position_linear_axe = 36000;
		conversionFactor = 1.81;
		positionTriggerStart = 1000;
		nbTriggers = 47;
		start_position_y_axe = 0;
		nb_steps = 20;
		return 0;
	}*/
	if (argc != 11) {
		cout << "Missing parameter" << endl;
		printUsage(argv);
		return -1;
	}
	for (int i = 1; i < argc; ++i) {
		if (string(argv[i]) == "--start_position_linear_axe") {
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
		else if (string(argv[i]) == "--start_position_y_axe") {
			if (legal_int(argv[i + 1]) == false) {
				cout << "Bad --start_position_y_axe flag value\n";
				return -1;
			}
			start_position_y_axe = atof(argv[i + 1]);
			i++;
		}
		else if (string(argv[i]) == "--end_position_y_axe") {
			if (legal_int(argv[i + 1]) == false) {
				cout << "Bad --end_position_y_axe flag value\n";
				return -1;
			}
			end_position_y_axe = atof(argv[i + 1]);
			i++;
		}
		else if (string(argv[i]) == "--current_position_y_axe") {
			if (legal_int(argv[i + 1]) == false) {
				cout << "Bad --current_position_y_axe flag value\n";
				return -1;
			}
			current_position_y_axe = atof(argv[i + 1]);
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
	cout << "start_position_y_axe " << start_position_y_axe << endl;
	cout << "end_position_y_axe " << end_position_y_axe << endl;
	cout << "current_position_y_axe " << current_position_y_axe << endl;

	if (end_position_linear_axe < start_position_linear_axe) {
		cout << "end position of linear axe must be more than srart position\n";
		return -1;
	}
	if (end_position_y_axe < start_position_y_axe) {
		cout << "end position of y axe must be more than srart position\n";
		return -1;
	}
	if (current_position_y_axe > end_position_y_axe || current_position_y_axe < start_position_y_axe) {
		cout << "current position of y axe must be less than end position and more than start position\n";
		return -1;
	}

	boost::property_tree::ptree pt;
	boost::property_tree::ini_parser::read_ini("settings.ini", pt);
	string tangohost = pt.get<std::string>("Hardware.tangohost");
	string cameraserver = pt.get<std::string>("Camera.cameraserver");
	string cameraport = pt.get<std::string>("Camera.cameraport");
	string path_to_pilc = tangohost + "/" + pt.get<std::string>("Hardware.pilc");
	//string path_to_linear_axis = tangohost + "/" + pt.get<std::string>("Hardware.linear_axis");
	//string path_to_y_axis = tangohost + "/" + pt.get<std::string>("Hardware.y_axis");
	//nbTriggers = pt.get<int>("ScanSettings.nbTriggers");
	//start_position_linear_axe = pt.get<double>("ScanSettings.start_position_linear_axe");
	//end_position_linear_axe = pt.get<double>("ScanSettings.end_position_linear_axe");
	//start_position_y_axe = pt.get<double>("ScanSettings.start_position_y_axe");
	positionTriggerStart = pt.get<double>("ScanSettings.positionTriggerStart");
	//nb_steps = pt.get<double>("ScanSettings.nb_steps");
	int triggerMode = pt.get<int>("ScanSettings.pilc_trigger_mode");
	conversionFactor = pt.get<double>("ScanSettings.conversionFactor");
	double downsamplingFactor = pt.get<double>("Frame.downsamplingFactor");

	Camera myCamera(cameraserver, cameraport);
	SizeOfImage sizeOfImage;
	sizeOfImage = myCamera.getSizeOfImage();
	int widthImage = sizeOfImage.width;
	int heightImage = sizeOfImage.height;

	//int widthBild = pt.get<int>("Frame.widthBild");
	//int heightBild = pt.get<int>("Frame.heightBild");
	int encoderNumber = pt.get<int>("ScanSettings.pilc_encoder_number");
	double tuning_parameter_linear_axe = pt.get<double>("Tuning.tuning_parameter_linear_axe");
	double tuning_parameter_y_axe = pt.get<double>("Tuning.tuning_parameter_y_axe");

	if (positionTriggerStart > (end_position_linear_axe - start_position_linear_axe)) {
		cout << "start position for pilc triggering is out of range\n";
		return -1;
	}
	int nbTriggers = (int) (end_position_linear_axe - start_position_linear_axe - positionTriggerStart) * conversionFactor / widthImage;

	int widthScene = (end_position_linear_axe - start_position_linear_axe + 2000) * conversionFactor / downsamplingFactor;
	int heightScene = (end_position_y_axe - start_position_y_axe + 2000) * conversionFactor / downsamplingFactor;
	Mat scene = Mat(heightScene, widthScene, CV_8UC3, Scalar(176, 226, 255));
	imwrite("scene.jpg", scene);
	
	vector<double> position1Data;
	vector<Mat> imageArray;

	pilc myPilc(path_to_pilc);

	myPilc.set_nbTriggers(nbTriggers);
	myPilc.set_positionTriggerStepSize(widthImage / conversionFactor);
	myPilc.set_positionTriggerStart(positionTriggerStart);
	myPilc.set_triggerMode(triggerMode);
	myPilc.calibrateEncoder(encoderNumber, 0);
	myCamera.switchStartFrameTriggerMode(myCamera.triggerMode);
	myPilc.set_arm(1);
	cout << "########## all parameters setted ##########" << endl;

	int counter = 0;
	while (counter < nbTriggers) {
		CaptureImage myCaptureImage(cameraserver, cameraport);
		Mat image = myCaptureImage.getImage();
		//imshow("Video", img1);
		//int key = waitKey(1);
		//cv::Mat dst;
		//cv::flip(img1, dst, 0);
		imageArray.push_back(image);
		counter++;
		//cout << "counter is: " << counter << endl;
		//ostringstream ss;
		//ss << counter;
		//string filename;
		//filename = ss.str() + ".jpg";
		//imwrite(filename, img1);
		//if (counter == nbTriggers) {break;}
	}
	cout << "########## finished scan ##########" << endl;

	myCamera.switchStartFrameTriggerMode(myCamera.fixedRate);
	position1Data = myPilc.get_position1Data();
	workImage myImage(imageArray, position1Data, widthImage, heightImage, conversionFactor, downsamplingFactor, tuning_parameter_linear_axe, tuning_parameter_y_axe);
	myImage.pasteImgToScene(current_position_y_axe, current_position_y_axe / start_position_y_axe);
	cout << "########## wrote scene ##########" << endl;

	imageArray.clear();
	position1Data.clear();

	return 0;
}
