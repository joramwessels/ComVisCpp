// ComVisCpp.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include "pch.h"

int main()
{
	//cv::Mat img = cv::imread("data/logo.png");
	//cv::namedWindow("OpenCV");
	//cv::imshow("OpenCV", img);
	//cv::waitKey(0);
	//return 0;

	cv::Size patternsize(8, 6); //interior number of corners
	cv::Mat gray = cv::imread("data/logo.png"); //source image
	std::vector<cv::Point2f> corners; //this will be filled by the detected corners

	//CALIB_CB_FAST_CHECK saves a lot of time on images
	//that do not contain any chessboard corners
	bool patternfound = findChessboardCorners(gray, patternsize, corners,
		cv::CALIB_CB_ADAPTIVE_THRESH + cv::CALIB_CB_NORMALIZE_IMAGE
		+ cv::CALIB_CB_FAST_CHECK);

	if (patternfound)
		cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
			cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

	//cv::InputOutputArray img = cv::InputOutputArray();
	cv::Mat img;
	cv::drawChessboardCorners(img, patternsize, cv::Mat(corners), patternfound);
	cv::namedWindow("OpenCV");
	cv::imshow("OpenCV", img);
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
