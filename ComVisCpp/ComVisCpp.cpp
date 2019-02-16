#include "pch.h"

using namespace std;
using namespace cv;

int main()
{
	const float calibrationSquareDimension = 0.022833f; // Meters
	const Size chessboardDimensions = Size(6, 9); // Number of intersections
	Mat gray = imread("data/checkerboard1.png");
	vector<Point2f> corners;

	//CALIB_CB_FAST_CHECK saves a lot of time on images
	//that do not contain any chessboard corners
	bool patternfound = findChessboardCorners(gray, chessboardDimensions, corners,
		CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE
		+ CALIB_CB_FAST_CHECK);

	//if (patternfound)
	//{
	//	cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
	//		cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
	//}

	Mat img = gray.clone();
	drawChessboardCorners(img, chessboardDimensions, Mat(corners), patternfound);

	for (int i = 0; i < corners.size(); i++) {
		printf("\n[%f, %f]", corners[i].x, corners[i].y);
	}

	namedWindow("OpenCV");
	imshow("OpenCV", img);
	waitKey(0);
	return 0;
}