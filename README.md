# LaneDetection
A small program that plays dashcam footage and identifies the lane markings using [opencv](https://github.com/opencv/opencv).

![LaneDetection](https://user-images.githubusercontent.com/52022661/211406243-f0fd5465-2fd9-402c-825a-1d0219cabec6.gif)

## The Algorithm
The bulk of the algorithm is dedicated to calculating a series of points along the lane lines with the last step connecting the points with line segments to form the finalized overlay.

* Grab the next frame from the video and convert it from RGB to HSV color space to leverage the intensity channel. Once in HSV space, a threshold is used to remove the low intensity pixels while leaving behind the high intensity pixels that make up the lane markings.

![HSV](https://user-images.githubusercontent.com/52022661/211678013-006decd7-ecfb-45e0-8f4d-400468c11c36.png)

* Use a perspective transform to obtain a bird’s eye view of the lanes.

![Bird'sEyeView](https://user-images.githubusercontent.com/52022661/211678227-459b13f8-6c9d-49b7-aa02-e616983e782d.png)

* A series of search boxes scanning from bottom to top average the positions of white pixels to form a point on the overlayed line. 
In order to determine where the points should go, a series of search boxes are used in which regions of the image are examined for white pixels. For all the white pixels that exist within a search box, their positions are first added up and then divided by the total white pixel count in order to obtain the average white pixel position. This average position will then be classified as a point that exists on one of the lane lines.
