#include "pch.h"

using namespace std;
using namespace cv;

int main()
{
	// Initializing image
	const float cellSize = 0.022833f; // Meters
	const Size boardDim = Size(6, 9); // Number of intersections
	Mat gray = imread("data/chessboard1.png");

	// Finding chessboard corners
	vector<Point2f> corners;
	if (!findChessboardCorners(gray, boardDim, corners, CALIB_CB_ADAPTIVE_THRESH
		+ CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK))
	{
		printf("No pattern found\n");
		return 1;
	}
	
	//cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
	Mat img = gray.clone();
	drawChessboardCorners(img, boardDim, Mat(corners), true);

	// The found corners, the projected coordinates
	vector<vector<Point2f>> imagePoints;
	imagePoints.push_back(corners);

	// The 3d world coordinates the corners represent
	vector<vector<Point3f>> objectPoints;
	vector<Point3f> object;
	for (int i = 0; i < boardDim.width; i++) for (int j = 0; j < boardDim.height; j++)
		object.push_back(Point3f(i * cellSize, j * cellSize, 0));
	objectPoints.push_back(object);

	// Calibrate the camera
	cv::Mat K; // The intrinsic paramters (camera matrix)
	cv::Mat D; // The distortion coefficients
	std::vector<cv::Mat> rvecs, tvecs; // The rotation and translation vectors
	calibrateCamera(objectPoints, imagePoints, gray.size(), K, D, rvecs, tvecs);

	if (false) drawAxesManually(K, rvecs, tvecs, &gray);
	else projectPoints(objectPoints, rvecs[0], tvecs[0], K, D, imagePoints);

	// Render to screen
	cv::namedWindow("OpenCV");
	cv::imshow("OpenCV", img);
	cv::waitKey(0);
	return 0;
}

// Manual axes drawing
void drawAxesManually(cv::Mat K, std::vector<cv::Mat> rvecs, std::vector<cv::Mat> tvecs, cv::Mat* img)
{
	// Transformation matrix
	cv::Mat R = rotationVectorToMatrix(rvecs[0]);		// Rotation matrix
	cv::Mat Rt = makeTransformationMatrix(R, tvecs[0]); // Extrinsic matrix

														// Unit points (column vectors: O, X, Y, Z)
	float unitP[16] =
	{ 0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
		1, 1, 1, 1 };
	cv::Mat unitPoints = cv::Mat(4, 4, CV_32F, unitP);

	// Projecting the unit vectors
	cv::Mat projP = K * Rt * unitPoints;
	cv::Mat projO = projP.col(0);
	cv::Mat projX = projP.col(1);
	cv::Mat projY = projP.col(2);
	cv::Mat projZ = projP.col(3);

	// Plotting projectedunit vectors
	cv::line(*img, cv::Point2f(projO.at<float>(0), projO.at<float>(1)), cv::Point2f(projX.at<float>(0), projX.at<float>(1)), 0xFF0000, 1);
	cv::line(*img, cv::Point2f(projO.at<float>(0), projO.at<float>(1)), cv::Point2f(projY.at<float>(0), projY.at<float>(1)), 0x00FF00, 1);
	cv::line(*img, cv::Point2f(projO.at<float>(0), projO.at<float>(1)), cv::Point2f(projZ.at<float>(0), projZ.at<float>(1)), 0x0000FF, 1);
}

// Converts the rotation vector into a rotation matrix
// https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle
cv::Mat rotationVectorToMatrix(cv::Mat rvec)
{
	float angle = (float) norm(rvec);
	cv::Mat u = rvec / angle;
	float ux = u.at<float>(0, 0), uy = u.at<float>(1, 0), uz = u.at<float>(2, 0);
	float cosT = cos(angle);
	float sinT = sin(angle);
	float matrix[9] = { ux * ux * (1 - cosT) + cos(angle),
						ux * uy * (1 - cosT) - uz * sinT,
						ux * uz * (1 - cosT) + uy * sinT,
						uy * ux * (1 - cosT) + uz * sinT,
						uy * uy * (1 - cosT) + cosT,
						uy * uz * (1 - cosT) - ux * sinT,
						uz * ux * (1 - cosT) - uy * sinT,
						uz * uy * (1 - cosT) + ux * sinT,
						uz * uz * (1 - cosT) + cosT };
	return cv::Mat(3, 3, CV_32F, matrix);
}

// Combines a rotation matrix R and translation vector t into an affine 4x3 transformation matrix
cv::Mat makeTransformationMatrix(cv::Mat R, cv::Mat t)
{
	float matrix[12] = { R.at<float>(0), R.at<float>(1), R.at<float>(2), t.at<float>(0),
						 R.at<float>(3), R.at<float>(4), R.at<float>(5), t.at<float>(1),
						 R.at<float>(6), R.at<float>(7), R.at<float>(8), t.at<float>(2) };
	return cv::Mat(4, 3, CV_32F, matrix);
}