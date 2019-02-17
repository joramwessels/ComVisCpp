#pragma once

void drawAxesManually(cv::Mat K, std::vector<cv::Mat> rvecs, std::vector<cv::Mat> tvecs, cv::Mat* img);
cv::Mat rotationVectorToMatrix(cv::Mat rvecs);
cv::Mat makeTransformationMatrix(cv::Mat R, cv::Mat t);