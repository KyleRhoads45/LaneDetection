#pragma once

#include <opencv2/opencv.hpp>

class VideoPlayer {
	
public:

	VideoPlayer(const char* filePath);
	cv::Mat Step();
	cv::Mat GetFrame();

private:

	cv::Mat frame;
	std::unique_ptr<cv::VideoCapture> capture;

};

