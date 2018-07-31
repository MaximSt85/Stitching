#ifndef CAMERA_H_
#define CAMERA_H_
#include <string>

struct SizeOfImage
{
     int width;
     int height;
};

class Camera {
public:
	Camera(std::string cameraserver, std::string cameraport);
	void switchStartFrameTriggerMode(int mode);
	int fixedRate = 3;
	int triggerMode = 1;
	int freerunMode = 0;
	std::string cameraserver;
	std::string cameraport;
	SizeOfImage getSizeOfImage();
private:

};
#endif
