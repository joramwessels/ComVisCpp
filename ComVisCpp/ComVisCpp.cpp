#include "pch.h"

using namespace std;
using namespace cv;

int main()
{
	// Initializing image
	const float cellSize = 0.022833f; // Meters
	const Size boardDim = Size(6, 9); // Number of intersections
	Mat gray = imread("data/chessboard1.png");

	// Calibrate the camera
	cv::Mat K; // The intrinsic paramters (camera matrix)
	cv::Mat D; // The distortion coefficients
	std::vector<cv::Mat> rvecs, tvecs; // The rotation and translation vectors

	// Import calibration images
	std::vector<cv::Mat> imageVector;
	for (int i = 1; i <= 17; i++) {
		imageVector.push_back(imread("data/chessboard" + to_string(i) + ".jpg"));
	}

	// Calibrate camera
	cameraCalibration(imageVector, boardDim, cellSize, K, D, rvecs, tvecs);
	//calibrateCamera(objectPoints, imagePoints, gray.size(), K, D, rvecs, tvecs);

	// Draw the axes on every image
	for (int i = 0; i < imageVector.size(); i++) {
		if (false) drawAxesManually(K, rvecs, tvecs, imageVector[i]);
		else drawAxes(imageVector[i], rvecs[i], tvecs[i], K, D);
	}

	return 0;
}

// Calculate the expected pixel locations of the inner corners of a chess board
void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners) {
	for (int y = 0; y < boardSize.height; y++) {
		for (int x = 0; x < boardSize.width; x++) {
			corners.push_back(Point3f(x * squareEdgeLength, y * squareEdgeLength, 0.0f));
		}
	}
}

// Find the pixel locations of the inner corners of a chess board
void getChessboardCorners(vector<Mat> images, Size boardSize, vector<vector<Point2f>>& allFoundPoints, bool showResults) {
	for (int i = 0; i < images.size(); i++) {
		vector<Point2f> foundPointBuffer;
		bool patternFound = findChessboardCorners(images[i], boardSize, foundPointBuffer, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK);

		if (patternFound) {
			allFoundPoints.push_back(foundPointBuffer);
		}

		if (showResults) {
			drawChessboardCorners(images[i], boardSize, foundPointBuffer, patternFound);
			imshow("Found corners", images[i]);
			waitKey(0);
		}
	}
}

// Calibrates the camera based on images that are confirmed to be suitable for camera calibration
void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distortionCoefficients, vector<Mat>& rvecs, vector<Mat>& tvecs, bool showResults) {
	vector<vector<Point2f>> foundPoints; // Pixel locations of the inner corners for each chess board image

	getChessboardCorners(calibrationImages, boardSize, foundPoints, showResults); // Populate the pixel locations vectors

	vector<vector<Point3f>> worldSpacePoints(1);
	createKnownBoardPosition(boardSize, squareEdgeLength, worldSpacePoints[0]);
	worldSpacePoints.resize(foundPoints.size(), worldSpacePoints[0]);

	cameraMatrix = Mat::eye(3, 3, CV_64F);
	distortionCoefficients = Mat::zeros(8, 1, CV_64F);

	double projectionError = calibrateCamera(worldSpacePoints, foundPoints, boardSize, cameraMatrix, distortionCoefficients, rvecs, tvecs);
	printf("Projection error: %f", projectionError);
}

// Prints out a matrix in a single column
void printMatrix(Mat matrix, string header) {
	if (header.compare("") != 0) {
		printf("%s: ", header);
	}

	uint16_t rows = matrix.rows;
	uint16_t columns = matrix.cols;
	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < columns; c++) {
			printf("%f", matrix.at<double>(r, c));
		}
	}
}

void drawAxes(Mat &inputImage, Mat rvecs, Mat tvecs, Mat cameraMatrix, Mat distMatrix) {
	vector<Point2d> imagePoints;
	vector<Point3d> objectPoints;

	objectPoints.push_back(Point3d(0, 0, 0));     // Origin
	objectPoints.push_back(Point3d(0.05, 0, 0));  // X axis
	objectPoints.push_back(Point3d(0, 0.05, 0));  // Y axis
	objectPoints.push_back(Point3d(0, 0, -0.05)); // Z axis

	Mat img = inputImage.clone();

	projectPoints(objectPoints, rvecs, tvecs, cameraMatrix, distMatrix, imagePoints);

	line(img, imagePoints[0], imagePoints[1], Scalar(255, 0, 0), 4);
	line(img, imagePoints[0], imagePoints[2], Scalar(0, 255, 0), 4);
	line(img, imagePoints[0], imagePoints[3], Scalar(0, 0, 255), 4);

	imshow("Axes", img);
	waitKey(0);
}

// Manual axes drawing (Joram's implementation; Legacy)
void drawAxesManually(cv::Mat K, std::vector<cv::Mat> rvecs, std::vector<cv::Mat> tvecs, cv::Mat img)
{
	const float cellSize = 0.022833f; // Meters
	const Size boardDim = Size(6, 9); // Number of intersections

	// Finding chessboard corners
	vector<Point2f> corners;
	if (!findChessboardCorners(img, boardDim, corners, CALIB_CB_ADAPTIVE_THRESH
		+ CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK))
	{
		printf("No pattern found\n");
		return;
	}

	//cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1), cv::TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));
	//Mat img = gray.clone();
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
	cv::line(img, cv::Point2f(projO.at<float>(0), projO.at<float>(1)), cv::Point2f(projX.at<float>(0), projX.at<float>(1)), 0xFF0000, 1);
	cv::line(img, cv::Point2f(projO.at<float>(0), projO.at<float>(1)), cv::Point2f(projY.at<float>(0), projY.at<float>(1)), 0x00FF00, 1);
	cv::line(img, cv::Point2f(projO.at<float>(0), projO.at<float>(1)), cv::Point2f(projZ.at<float>(0), projZ.at<float>(1)), 0x0000FF, 1);

	// Render to screen
	cv::namedWindow("OpenCV");
	cv::imshow("OpenCV", img);
	cv::waitKey(0);
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