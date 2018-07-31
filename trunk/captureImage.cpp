#include "captureImage.h"

using namespace std;
using namespace cv;

CaptureImage::CaptureImage(string cameraserver, string cameraport) {
	url = "http://" + cameraserver + ":" + cameraport + "/?action=snapshot/frame.mjpg";
	chunk.memory = (char*)malloc(1);
	chunk.size = 0;
}

Mat CaptureImage::getImage() {
	CURL *curl_handle;
	CURLcode res;
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url.c_str());
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void * )&chunk);
	res = curl_easy_perform(curl_handle);

	if (res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
	} else {
		//printf("%lu bytes retrieved\n", (long) chunk.size);
		image = imdecode(Mat(1, chunk.size, CV_8UC3, chunk.memory), CV_LOAD_IMAGE_UNCHANGED);
		/*namedWindow("Image", CV_WINDOW_AUTOSIZE);
		if (!image.empty()) {
			imshow("Image", image);
			waitKey();
		}*/
	}

	curl_easy_cleanup(curl_handle);
	if (chunk.memory) {
		free(chunk.memory);
	}
	curl_global_cleanup();

	return image;
}
