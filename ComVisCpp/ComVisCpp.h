#pragma once

void createKnownBoardPosition(cv::Size boardSize, float squareEdgeLength, std::vector<cv::Point3f>& corners);
void getChessboardCorners(std::vector<cv::Mat> images, cv::Size boardSize, std::vector<std::vector<cv::Point2f>>& allFoundPoints, bool showResults = false);
void cameraCalibration(std::vector<cv::Mat> calibrationImages, cv::Size boardSize, float squareEdgeLength, cv::Mat& cameraMatrix, cv::Mat& distortionCoefficients, std::vector<cv::Mat>& rvecs, std::vector<cv::Mat>& tvecs, bool showResults = false);
void printMatrix(cv::Mat matrix, std::string header = "");
void drawAxes(cv::Mat &inputImage, cv::Mat rvecs, cv::Mat tvecs, cv::Mat cameraMatrix, cv::Mat distMatrix);

void drawAxesManually(cv::Mat K, std::vector<cv::Mat> rvecs, std::vector<cv::Mat> tvecs, cv::Mat img);
cv::Mat rotationVectorToMatrix(cv::Mat rvecs);
cv::Mat makeTransformationMatrix(cv::Mat R, cv::Mat t);