#include "LaneDetection.h"

int main() {

	std::unique_ptr<LaneDetection> laneDetect = std::make_unique<LaneDetection>();

	while (laneDetect->IsRunning()) {
		laneDetect->Update();
	}

	return 0;
}
