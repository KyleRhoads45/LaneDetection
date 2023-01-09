#pragma once

#include <opencv2/opencv.hpp>
#include "VideoPlayer.h"

class LaneDetection {

public:

	LaneDetection();

	bool IsRunning();
	void Update();

	cv::Mat ConvertToHSV(cv::Mat input);
	cv::Mat IsolateLaneColors(cv::Mat input);
	cv::Mat ConvertToBinary(cv::Mat input);
	cv::Mat TopDownTransformPerspective(cv::Mat input);

	int* GetStartX(cv::Mat input);
	std::vector<cv::Point2f> CalculatePoints(cv::Mat input, int xPeakValues);
	std::vector<cv::Point2f> ReverseTransformPoints(std::vector<cv::Point2f> points);
	void OverlayLaneLine(cv::Mat frame, std::vector<cv::Point2f> points);
	void DisplayFrame(cv::Mat frame);

private:
	
	std::unique_ptr<VideoPlayer> vPlayer;

};

