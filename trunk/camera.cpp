#include "camera.h"

#include <iostream>
#include <string>
#include <iomanip>
#include <sstream>

#include <curlpp/cURLpp.hpp>
#include <curlpp/Easy.hpp>
#include <curlpp/Options.hpp>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

using namespace curlpp::options;
using namespace std;


Camera::Camera(string cameraserver1, string cameraport1) {
	cameraserver = cameraserver1;
	cameraport = cameraport1;
}

void Camera::switchStartFrameTriggerMode(int mode){
	try {
		curlpp::Cleanup myCleanup;
		curlpp::Easy myRequest;
		if (mode == 0) {
			//myRequest.setOpt<Url>("http://192.168.39.1:8082/?action=command&id=13&dest=0&group=42&value=0&plugin=0");
			myRequest.setOpt<Url>("http://" + cameraserver + ":" + cameraport + "/?action=command&id=13&dest=0&group=42&value=0&plugin=0");
		}
		else if (mode == 1) {
			//myRequest.setOpt<Url>("http://192.168.39.1:8082/?action=command&id=13&dest=0&group=42&value=1&plugin=0");
			myRequest.setOpt<Url>("http://" + cameraserver + ":" + cameraport + "/?action=command&id=13&dest=0&group=42&value=1&plugin=0");
		}
		else if (mode == 3) {
			//myRequest.setOpt<Url>("http://192.168.39.1:8082/?action=command&id=13&dest=0&group=42&value=3&plugin=0");
			myRequest.setOpt<Url>("http://" + cameraserver + ":" + cameraport + "/?action=command&id=13&dest=0&group=42&value=3&plugin=0");
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

SizeOfImage Camera::getSizeOfImage(){
	stringstream ss;
	curlpp::Cleanup myCleanup;
	curlpp::Easy myRequest;
	myRequest.setOpt<Url>("http://" + cameraserver + ":" + cameraport + "/input_0.json");
	myRequest.perform();
	ss << myRequest;
	boost::property_tree::ptree pt;
    boost::property_tree::read_json(ss, pt);
	int width;
	int height;
	BOOST_FOREACH(boost::property_tree::ptree::value_type &v, pt.get_child("controls")) {
		string name = v.second.get<string>("name", "");
		if (!name.compare("Width")) {
			width = v.second.get<int>("value", 0);
		}
		if (!name.compare("Height")) {
			height = v.second.get<int>("value", 0);
		}
	}
	//std::cout << "width is: " << width << std::endl;
	//std::cout << "height is: " << height << std::endl;
	//string response;
	//response = ss.str();
	SizeOfImage result = {width, height};
	return result;
}
