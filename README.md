# LaneDetection
A small program that plays dashcam footage and identifies the lane markings using [opencv](https://github.com/opencv/opencv).

![LaneDetection](https://user-images.githubusercontent.com/52022661/211406243-f0fd5465-2fd9-402c-825a-1d0219cabec6.gif)

## The Algorithm
The bulk of the algorithm is dedicated to calculating a series of points along the lane lines with the last step connecting the points with line segments to form the finalized overlay.

* Grab the next frame from the video and convert it from RGB to HSV color space to leverage the intensity channel. Once in HSV space, a threshold is used to remove the low intensity pixels while leaving behind the high intensity pixels that make up the lane markings.

![HSV](https://user-images.githubusercontent.com/52022661/211678013-006decd7-ecfb-45e0-8f4d-400468c11c36.png)

* Use a perspective transform to obtain a bird’s eye view of the lanes.

![Bird'sEyeView](https://user-images.githubusercontent.com/52022661/211678227-459b13f8-6c9d-49b7-aa02-e616983e782d.png)

* Using a series of search boxes scanning from bottom to top, average the positions of white pixels within a search box to form a point on the overlayed line. When placing a search box, offset it using the distance between the previous search box's center and its average white pixel point. This ensures that the search boxes follow the curvature of the lane lines.

![SlidingBoxes](https://user-images.githubusercontent.com/52022661/211681820-6d40ec76-7926-42a6-b941-00ecffa3e286.png)

* Reverse the perspective transform on the points and connect them with line segments.

![FinalResult](https://user-images.githubusercontent.com/52022661/211682068-d3262190-7236-48c6-8281-27bc0e980d54.png)
