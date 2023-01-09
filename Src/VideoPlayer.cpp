#include "VideoPlayer.h"

VideoPlayer::VideoPlayer(const char* filePath) {
	capture = std::make_unique<cv::VideoCapture>(filePath);
}

cv::Mat VideoPlayer::Step() {
	capture->read(frame);
	return frame;
}

cv::Mat VideoPlayer::GetFrame() {
	return frame;
}

