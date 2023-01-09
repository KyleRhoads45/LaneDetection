#include "LaneDetection.h"

LaneDetection::LaneDetection() {
	const char* videoFile = "DrivingClips.mov";
	vPlayer = std::make_unique<VideoPlayer>(videoFile);
}

bool LaneDetection::IsRunning() {
	const int escKey = 27;
	if (cv::pollKey() == escKey) {
		return false;
	}
	return true;
}

void LaneDetection::Update() {

	//This is the main function


	//Get the next frame in the video
	cv::Mat img = vPlayer->Step();

	//If the video is over just return to prevent
	//the program from crashing 
	if(img.empty()) {
		return;
	}

	//Image processing methods
	img = ConvertToHSV(img);
	img = IsolateLaneColors(img);
	img = TopDownTransformPerspective(img);
	img = ConvertToBinary(img);

	//Find the x values for the starting positions of the search boxes
	//Returns an int[] of size 2
	int* startXs = GetStartX(img);

	//Calculate the points
	std::vector<cv::Point2f> leftPoints = CalculatePoints(img, startXs[0]);
	std::vector<cv::Point2f> rightPoints = CalculatePoints(img, startXs[1]);

	//Reverse the perspective transform so the points get projected onto the lane lines
	leftPoints = ReverseTransformPoints(leftPoints);
	rightPoints = ReverseTransformPoints(rightPoints);

	//Draw the lane overlays
	cv::Mat frame = vPlayer->GetFrame();
	OverlayLaneLine(frame, leftPoints);
	OverlayLaneLine(frame, rightPoints);

	//Display the final result
	DisplayFrame(frame);
}

cv::Mat LaneDetection::ConvertToHSV(cv::Mat input) {
	cv::Mat output;
	cv::cvtColor(input, output, cv::COLOR_BGR2HSV);
	return output;
}

cv::Mat LaneDetection::IsolateLaneColors(cv::Mat input) {
	cv::Mat output;
	cv::inRange(input, cv::Scalar(0, 0, 128), cv::Scalar(150, 255, 255), output);
	return output;
}

cv::Mat LaneDetection::ConvertToBinary(cv::Mat input) {
	cv::Mat output;
	cv::threshold(input, output, 0, 255, cv::THRESH_BINARY);
	return output;
}

cv::Mat LaneDetection::TopDownTransformPerspective(cv::Mat input) {
	cv::Point2f srcPoints[4];
	srcPoints[1] = cv::Point(655, 680);			//Top Left
	srcPoints[3] = cv::Point(1150, 680);		//Top Right
	srcPoints[0] = cv::Point(0, 1080);			//Bottom Left
	srcPoints[2] = cv::Point(1920, 1080);		//Bottom Right

	cv::Point2f destPoints[4];
	destPoints[1] = cv::Point(0, 0);			//Top Left
	destPoints[3] = cv::Point(1920, 0);			//Top Right
	destPoints[0] = cv::Point(0, 1080);			//Bottom Left
	destPoints[2] = cv::Point(1920, 1080);		//Bottom Right

	cv::Mat trans = cv::getPerspectiveTransform(srcPoints, destPoints);
	cv::Mat output;
	cv::warpPerspective(input, output, trans, cv::Size(1920, 1080));
	return output;
}

int* LaneDetection::GetStartX(cv::Mat input) {
	int hist[1920] = { 0 };
	for (int i = 0; i < input.cols; i++) {
		for (int j = 0; j < input.rows; j++) {
			int intensity = input.at<uchar>(j, i);
			if (intensity == 255) {
				hist[i]++;
			}
		}
	}

	int largestPeaks[2] = { 0 };
	int* largestPeakIndices = new int[2];

	for (int i = 0; i < 1920; i++) {
		int index = (i < 960) ? 0 : 1;
		int val = hist[i];
		if (val > largestPeaks[index]) {
			largestPeaks[index] = val;
			largestPeakIndices[index] = i;
		}
	}

	return largestPeakIndices;
}

std::vector<cv::Point2f> LaneDetection::CalculatePoints(cv::Mat input, int startX) {
	const int segmentCount = 30;
	const int searchHeight = 1080 / segmentCount;
	const int searchWidth = searchHeight * 4;
	const int startYPos = 1080 - 1;

	int xPos = startX;
	std::vector<cv::Point2f> points;

	//Slide boxes
	for (int i = 0; i < segmentCount; i++) {
		cv::Point2f point1(xPos - searchWidth, startYPos - (searchHeight * i));
		cv::Point2f point2(xPos + searchWidth, startYPos - (searchHeight * (i + 1)));

		std::vector<cv::Point2f> segmentPoints;
		for (int y = point1.y; y > point2.y; y--) {
			if(y < 0 || y >= 1080) {
				continue;
			}
			for (int x = point1.x; x <= point2.x; x++) {
				if(x < 0 || x >= 1920) {
					continue;
				}
				if (input.at<uchar>(y, x) == 255) {
					segmentPoints.push_back(cv::Point2f(x, y));
				}
			}
		}

		if (segmentPoints.empty()) {
			continue;
		}

		//Calculate the average white pixel position

		cv::Point2f avgPoint = segmentPoints[0];
		for (int j = 1; j < segmentPoints.size(); j++) {
			avgPoint += segmentPoints[j];
		}

		avgPoint.x /= segmentPoints.size();
		avgPoint.y /= segmentPoints.size();

		int xOffset = avgPoint.x - xPos;
		xPos += xOffset;
			
		points.push_back(avgPoint);
	}

	return points;

}

std::vector<cv::Point2f> LaneDetection::ReverseTransformPoints(std::vector<cv::Point2f> points) {
	if (!points.empty()) {
		cv::Point2f srcPoints[4];
		srcPoints[1] = cv::Point(655, 680);			//Top Left
		srcPoints[3] = cv::Point(1150, 680);		//Top Right
		srcPoints[0] = cv::Point(0, 1080);			//Bottom Left
		srcPoints[2] = cv::Point(1920, 1080);		//Bottom Right

		cv::Point2f destPoints[4];
		destPoints[1] = cv::Point(0, 0);			//Top Left
		destPoints[3] = cv::Point(1920, 0);			//Top Right
		destPoints[0] = cv::Point(0, 1080);			//Bottom Left
		destPoints[2] = cv::Point(1920, 1080);		//Bottom Right

		cv::Mat trans = cv::getPerspectiveTransform(destPoints, srcPoints);
		cv::perspectiveTransform(points, points, trans);
	}
	return points;
}

void LaneDetection::OverlayLaneLine(cv::Mat frame, std::vector<cv::Point2f> points) {

	//If there is not enough data, ie dashed lane lines,
	//interpolate where the lane is
	const int minPointsNeeded = 30;
	if(points.size() < minPointsNeeded) {
		cv::Point2f startPoint = points[0];
		cv::Point2f endPoint = points[points.size() - 1];

		float m = (startPoint.y - endPoint.y) / (startPoint.x - endPoint.x);

		const int minYPos = 1079;
		const int maxYPos = 680;
		int bottomX = ((minYPos - startPoint.y) / m) + startPoint.x;
		int topX = ((maxYPos - startPoint.y) / m) + startPoint.x;

		cv::Point2f interpStartPoint(bottomX, minYPos);
		cv::Point2f interpEndPoint(topX, maxYPos);
		cv::line(frame, interpStartPoint, interpEndPoint, cv::Scalar(255, 0, 0), 10, 8, 0);

		return;
	}
	
	//Calculate where the very bottom point because the first average pixel point will
	//not be at the bottom of the image, so as to prevent a gap.
	float m = (points[0].y - points[1].y) / (points[0].x - points[1].x);
	int x = ((1079 - points[0].y) / m) + points[0].x;
	cv::Point2f interpStartPoint(x, 1079);


	//Draw lane line
	cv::line(frame, interpStartPoint, points[0], cv::Scalar(255, 0, 0), 10, 8, 0);
	for (int i = 1; i < points.size(); i++) {
		cv::line(frame, points[i - 1], points[i], cv::Scalar(255, 0, 0), 10, cv::LINE_AA, 0);
	}

}

void LaneDetection::DisplayFrame(cv::Mat frame) {
	cv::resize(frame, frame, cv::Size(1600, 900));
	cv::imshow("Lane Detection", frame);
}

