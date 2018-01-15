// HittaValle.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2\ml.hpp>
#include <opencv2\ml\ml.hpp>


#include<iostream>
#include<conio.h>           // may have to modify this line if not using Windows

using namespace cv;
using namespace cv::ml;

int main() {
	// Data for visual representation
	int width = 512, height = 512;

	cv::Mat input, labels;// = cv::Mat(width, height, CV_32FC4);
	for (int i = 0; i < 8; ++i)
	{
		std::string filePath = "../Resources/64/notwaldo/1_0_" + std::to_string(i) + ".jpg";
		Mat file = cv::imread(filePath);
		file = file.reshape(1, 1);
		file.convertTo(file, CV_32F);
		input.push_back(file);

		labels.push_back(-1);
	}
	//for (int i = 6; i < 8; ++i)
	{
		std::string filePath = "../Resources/64-bw/waldo/1_4_6.jpg"; //+ std::to_string(i) + ".jpg";
		Mat file = cv::imread(filePath);
		file = file.reshape(1, 1);
		file.convertTo(file, CV_32F);
		input.push_back(file);

		labels.push_back(1);
	}


	cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();

	svm->setType(cv::ml::SVM::C_SVC);
	svm->setKernel(cv::ml::SVM::LINEAR);
	svm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, 100, 1e-6));
	
	cv::Ptr<ml::TrainData> traindata = cv::ml::TrainData::create(input, ml::ROW_SAMPLE, labels);

	svm->train(traindata);

	cv::Mat test = cv::imread("../Resources/64-bw/waldo/1_4_7.jpg");
	test = test.reshape(1, 1);
	test.convertTo(test, CV_32F);

	float what = svm->predict(test);

	int stopper = 0;

	/*cv::Mat image = cv::Mat::zeros(height, width, CV_8UC3);
	// Set up training data
	//int labels[4] = { 1, 1, -1, 0 };
	//float trainingData[4][2] = { { 10, 10 },{ 255, 125 },{ 501, 255 },{ 10, 501 } };


	//cv::Mat trainingDataMat(4, 2, CV_32FC1, trainingData);
	//cv::Mat labelsMat(4, 1, CV_32SC1, labels);
	
	cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();
	
	svm->setType(cv::ml::SVM::C_SVC);
	svm->setKernel(cv::ml::SVM::LINEAR);
	svm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, 1000, 1e-6));
	//svm->train(trainingDataMat, cv::ml::ROW_SAMPLE, labelsMat);

	//svm->train(ml::TrainData::create(input, ml::ROW_SAMPLE, labels));

	cv::Vec3b green(0, 255, 0), blue(255, 0, 0);
	for (int i = 0; i < image.rows; ++i)
		for (int j = 0; j < image.cols; ++j)
		{
			cv::Mat sampleMat = (cv::Mat_<float>(1, 2) << j, i);
			float response = svm->predict(sampleMat);
			if (response == 1)
				image.at<cv::Vec3b>(i, j) = green;
			else if (response == -1)
				image.at<cv::Vec3b>(i, j) = blue;
		}
	// Show the training data
	int thickness = 1;
	int lineType = 8;
	circle(image, Point(10, 10), 10, Scalar(255, 255, 255), thickness, lineType);
	circle(image, Point(255, 125), 5, Scalar(255, 255, 255), thickness, lineType);
	circle(image, Point(501, 255), 5, Scalar(255, 255, 255), thickness, lineType);
	circle(image, Point(10, 501), 5, Scalar(255, 255, 255), thickness, lineType);
	// Show support vectors
	thickness = 2;
	lineType = 8;
	Mat sv = svm->getUncompressedSupportVectors();
	for (int i = 0; i < sv.rows; ++i)
	{
		const float* v = sv.ptr<float>(i);
		circle(image, Point((int)v[0], (int)v[1]), 6, Scalar(128, 128, 128), thickness, lineType);
	}
	imwrite("result.png", image);        // save the image*/
	
	imshow("SVM Simple Example", input); // show it to the user
	
	cv::waitKey(0); 

	return(0);
}

