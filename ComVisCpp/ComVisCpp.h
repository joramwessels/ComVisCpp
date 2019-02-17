#pragma once

void createKnownBoardPosition(Size boardSize, float squareEdgeLength, vector<Point3f>& corners);
void getChessboardCorners(vector<Mat> images, Size boardSize, vector<vector<Point2f>>& allFoundPoints, bool showResults = false);
void cameraCalibration(vector<Mat> calibrationImages, Size boardSize, float squareEdgeLength, Mat& cameraMatrix, Mat& distortionCoefficients, vector<Mat>& rvecs, vector<Mat>& tvecs, bool showResults = false);
void printMatrix(Mat matrix, string header = "");
void drawAxes(Mat &inputImage, Mat rvecs, Mat tvecs, Mat cameraMatrix, Mat distMatrix);

void drawAxesManually(cv::Mat K, std::vector<cv::Mat> rvecs, std::vector<cv::Mat> tvecs, cv::Mat img);
cv::Mat rotationVectorToMatrix(cv::Mat rvecs);
cv::Mat makeTransformationMatrix(cv::Mat R, cv::Mat t);