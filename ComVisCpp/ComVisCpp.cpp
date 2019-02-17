#include "pch.h"

using namespace std;
using namespace cv;

// Calculate the expected pixel locations of the inner corners of a chess board
void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners) {
	for (int y = 0; y < boardSize.height; y++) {
		for (int x = 0; x < boardSize.width; x++) {
			corners.push_back(Point3f(x * squareEdgeLength, y * squareEdgeLength, 0.0f));
		}
	}
}

// Find the pixel locations of the inner corners of a chess board
void getChessboardCorners(vector<Mat> images, Size boardSize, vector<vector<Point2f>>& allFoundPoints, bool showResults = false) {
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
void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distortionCoefficients, vector<Mat>& rvecs, vector<Mat>& tvecs, bool showResults = false) {
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
void printMatrix(Mat matrix, string header = "") {
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

int main()
{
	vector<Mat> imageVector;
	const Size chessboardDimensions = Size(6, 9); // Number of intersections
	const float calibrationSquareDimension = 0.022833f; // Meters
	Mat calibratedCameraMatrix;
	Mat calibratedDistortionCoefficients;
	vector<Mat> calibratedRotationVectors;
	vector<Mat> calibratedTranslationVectors;

	// Import calibration images
	for (int i = 1; i <= 17; i++) {
		imageVector.push_back(imread("data/chessboard" + to_string(i) + ".jpg"));
	}

	// Calibrate camera
	cameraCalibration(imageVector, chessboardDimensions, calibrationSquareDimension, calibratedCameraMatrix, calibratedDistortionCoefficients, calibratedRotationVectors, calibratedTranslationVectors);

	// Draw the axes on every image
	for (int i = 0; i < imageVector.size(); i++) {
		drawAxes(imageVector[i], calibratedRotationVectors[i], calibratedTranslationVectors[i], calibratedCameraMatrix, calibratedDistortionCoefficients);
	}

	return 0;
}