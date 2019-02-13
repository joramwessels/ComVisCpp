#include "pch.h"

int main()
{
	cv::Size patternsize(8, 6);					//interior number of corners
	cv::Mat gray = cv::imread("data/logo.png"); //source image
	std::vector<cv::Point2f> corners;			//this will be filled by the detected corners

	//CALIB_CB_FAST_CHECK saves a lot of time on images
	//that do not contain any chessboard corners
	bool patternfound = findChessboardCorners(gray, patternsize, corners,
		cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE
		+ cv::CALIB_CB_FAST_CHECK);

	if (patternfound)
	{
		cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
			cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
	}

	cv::drawChessboardCorners(gray, patternsize, cv::Mat(corners), patternfound);
	cv::namedWindow("OpenCV");
	cv::imshow("OpenCV", gray);
	cv::waitKey(0);
	return 0;
}