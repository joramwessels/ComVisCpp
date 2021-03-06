#include "pch.h"

using namespace std;
using namespace cv;

int main()
{
	// Initializing image
	const float cellSize = 0.022833f; // Meters
	const Size boardDim = Size(6, 9); // Number of intersections

	// Import calibration images
	std::vector<cv::Mat> imageVector;
	for (int i = 18; i <= 20; i++) { // DEBUG 60 -> 19
		imageVector.push_back(imread("data/chessboard" + to_string(i) + ".jpg"));
	}

	// Calibrate the camera
	cv::Mat K; // The intrinsic paramters (camera matrix)
	cv::Mat D; // The distortion coefficients
	std::vector<cv::Mat> rvecs, tvecs; // The rotation and translation vectors
	cameraCalibration(imageVector, boardDim, cellSize, K, D, rvecs, tvecs);

	// Draw the axes on every image
	for (int i = 0; i < imageVector.size(); i++) {
		if (true) drawAxesManually(K, rvecs[i], tvecs[i], imageVector[i], boardDim, cellSize);
		else drawAxes(imageVector[i], rvecs[i], tvecs[i], K, D);
	}

	return 0;
}

// Calculate the expected pixel locations of the inner corners of a chess board
void createKnownBoardPosition(cv::Size boardSize, float squareEdgeLength, std::vector<Point3f>& corners) {
	for (int y = 0; y < boardSize.height; y++) {
		for (int x = 0; x < boardSize.width; x++) {
			corners.push_back(Point3f(x * squareEdgeLength, y * squareEdgeLength, 0.0f));
		}
	}
}

// Find the pixel locations of the inner corners of a chess board
// Takes in a vector of images and uses cv::findChessboardCorners to find the corners in each of those
void getChessboardCorners(std::vector<cv::Mat> images, cv::Size boardSize, std::vector<std::vector<Point2f>>& allFoundPoints, bool showResults) {
	for (int i = 0; i < images.size(); i++) {
		std::vector<Point2f> foundPointBuffer;
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
// It uses the multiple images to improve the camera intrinsics approximation
void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distortionCoefficients, vector<Mat>& rvecs, vector<Mat>& tvecs, bool showResults) {
	vector<vector<Point2f>> foundPoints; // Pixel locations of the inner corners for each chess board image

	getChessboardCorners(calibrationImages, boardSize, foundPoints, showResults); // Populate the pixel locations vectors

	vector<vector<Point3f>> worldSpacePoints(1);
	createKnownBoardPosition(boardSize, squareEdgeLength, worldSpacePoints[0]);
	worldSpacePoints.resize(foundPoints.size(), worldSpacePoints[0]);

	cameraMatrix = Mat::eye(3, 3, CV_64F);
	distortionCoefficients = Mat::zeros(8, 1, CV_64F);

	double projectionError = calibrateCamera(worldSpacePoints, foundPoints, boardSize, cameraMatrix, distortionCoefficients, rvecs, tvecs);
	printf("\nProjection error: %f\n", projectionError);
}

// Prints out a matrix in a single column
void printMatrix(Mat matrix, string header) {
	if (header.compare("") != 0) {
		printf("\n\n%s: ", header);
	}

	uint16_t rows = matrix.rows;
	uint16_t columns = matrix.cols;
	for (int r = 0; r < rows; r++) {
		for (int c = 0; c < columns; c++) {
			printf("\n%f", matrix.at<double>(r, c));
		}
	}
}

// Given a set of calibration variables, draws x, y, and z axes at the origin of a chessboard
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

// Given a set of calibration variables, draws a 1x1 (based on cell size) cube at the origin of a chessboard
void drawCube(Mat &inputImage, float dimension, Mat rvecs, Mat tvecs, Mat cameraMatrix, Mat distMatrix) {
	vector<Point2d> imagePoints;
	vector<Point3d> objectPoints;

	objectPoints.push_back(Point3d(0, 0, 0));
	objectPoints.push_back(Point3d(dimension, 0, 0));
	objectPoints.push_back(Point3d(dimension, dimension, 0));
	objectPoints.push_back(Point3d(dimension, 0, -dimension));
	objectPoints.push_back(Point3d(0, dimension, 0));
	objectPoints.push_back(Point3d(0, dimension, -dimension));
	objectPoints.push_back(Point3d(0, 0, -dimension));
	objectPoints.push_back(Point3d(dimension, dimension, -dimension));

	Mat img = inputImage.clone();

	projectPoints(objectPoints, rvecs, tvecs, cameraMatrix, distMatrix, imagePoints);

	line(img, imagePoints[0], imagePoints[1], Scalar(0, 255, 0), 1);
	line(img, imagePoints[0], imagePoints[4], Scalar(0, 255, 0), 1);
	line(img, imagePoints[1], imagePoints[2], Scalar(0, 255, 0), 1);
	line(img, imagePoints[2], imagePoints[4], Scalar(0, 255, 0), 1);
	line(img, imagePoints[0], imagePoints[6], Scalar(0, 255, 0), 1);
	line(img, imagePoints[4], imagePoints[5], Scalar(0, 255, 0), 1);
	line(img, imagePoints[2], imagePoints[7], Scalar(0, 255, 0), 1);
	line(img, imagePoints[1], imagePoints[3], Scalar(0, 255, 0), 1);
	line(img, imagePoints[6], imagePoints[3], Scalar(0, 255, 0), 1);
	line(img, imagePoints[3], imagePoints[7], Scalar(0, 255, 0), 1);
	line(img, imagePoints[6], imagePoints[5], Scalar(0, 255, 0), 1);
	line(img, imagePoints[5], imagePoints[7], Scalar(0, 255, 0), 1);

	imshow("Axes", img);
	waitKey(0);
}

// Manual axes drawing (Joram's implementation; Legacy)
void drawAxesManually(cv::Mat K, cv::Mat rvec, cv::Mat tvec, cv::Mat img, cv::Size boardDim, float cellSize)
{
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
	cv::Mat R = rotationVectorToMatrix(rvec);		// Rotation matrix
	cv::Mat Rt = makeTransformationMatrix(R, tvec); // Extrinsic matrix

	// Unit points (column vectors: O, X, Y, Z)
	double sz = (double) cellSize * 3;
	double unitP[16] = {
		0.0, sz, 0.0, 0.0,
		0.0, 0.0, sz, 0.0,
		0.0, 0.0, 0.0, sz,
		1.0, 1.0, 1.0, 1.0
	};
	cv::Mat unitPoints = cv::Mat(4, 4, CV_64F, unitP);

	printf("\n");
	printOpenCVMatrix(K);
	printOpenCVMatrix(R);
	printOpenCVMatrix(tvec);
	printOpenCVMatrix(Rt);
	printOpenCVMatrix(unitPoints);

	// Projecting the unit vectors
	cv::Mat projP = K * Rt * unitPoints;
	cv::Mat projO = projP.col(0) / projP.at<double>(2, 0);
	cv::Mat projX = projP.col(1) / projP.at<double>(2, 1);
	cv::Mat projY = projP.col(2) / projP.at<double>(2, 2);
	cv::Mat projZ = projP.col(3) / projP.at<double>(2, 3);

	printOpenCVMatrix(projO);
	printOpenCVMatrix(projX);
	printOpenCVMatrix(projY);
	printOpenCVMatrix(projZ);

	// Plotting projectedunit vectors
	if (projP.type() != CV_64F) printf("Type mismatch in projected points\n");
	cv::line(img, cv::Point2f(projO.at<double>(0), projO.at<double>(1)), cv::Point2f(projX.at<double>(0), projX.at<double>(1)), cv::Scalar(0, 0, 255), 4);
	cv::line(img, cv::Point2f(projO.at<double>(0), projO.at<double>(1)), cv::Point2f(projY.at<double>(0), projY.at<double>(1)), cv::Scalar(0, 255, 0), 4);
	cv::line(img, cv::Point2f(projO.at<double>(0), projO.at<double>(1)), cv::Point2f(projZ.at<double>(0), projZ.at<double>(1)), cv::Scalar(255, 0, 0), 4);

	// Render to screen
	cv::namedWindow("OpenCV");
	cv::imshow("OpenCV", img);
	cv::waitKey(0);
}

// Converts the rotation vector into a rotation matrix
// https://en.wikipedia.org/wiki/Rotation_matrix#Rotation_matrix_from_axis_and_angle
cv::Mat rotationVectorToMatrix(cv::Mat rvec)
{
	if (rvec.type() != CV_64F) printf("Type mismatch in rotationVectorToMatrix()\n");
	double angle = norm(rvec);
	cv::Mat u = rvec / angle;
	double cosT = cos(angle);
	double sinT = sin(angle);
	double ux = u.ptr<double>(0)[0], uy = u.ptr<double>(1)[0], uz = u.ptr<double>(2)[0];
	double matrix[9] = {
		ux * ux * (1 - cosT) + cos(angle),
		ux * uy * (1 - cosT) - uz * sinT,
		ux * uz * (1 - cosT) + uy * sinT,
		uy * ux * (1 - cosT) + uz * sinT,
		uy * uy * (1 - cosT) + cosT,
		uy * uz * (1 - cosT) - ux * sinT,
		uz * ux * (1 - cosT) - uy * sinT,
		uz * uy * (1 - cosT) + ux * sinT,
		uz * uz * (1 - cosT) + cosT
	};
	return cv::Mat(3, 3, CV_64F, matrix).t();
}

// Combines a rotation matrix R and translation vector t into an affine 4x3 transformation matrix
cv::Mat makeTransformationMatrix(cv::Mat R, cv::Mat t)
{
	if (t.type() != CV_64F) printf("Type mismatch in makeTransformationMatrix()\n");
	cv::Mat Rt;
	cv::hconcat(R, t, Rt);
	return Rt;
}

void printOpenCVMatrix(cv::Mat m)
{
	if (m.type() != CV_64F) printf("Type: %i\n", m.type());
	for (int i = 0; i < m.rows; i++)
	{
		const double* ptr = m.ptr<double>(i);
		for (int j = 0; j < m.cols; j++)
		{
			printf("%.1f\t", ptr[j]);
		}
		printf("\n");
	}
	printf("\n");
}