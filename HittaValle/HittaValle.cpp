// HittaValle.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2\ml.hpp>
#include <opencv2\ml\ml.hpp>

#include <iostream>
#include <conio.h>           // may have to modify this line if not using Windows


void GenerateDataset(cv::Mat &dataset, cv::Mat &labels);

inline bool FileExists(std::string name)
{
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

const enum ImgSize
{
	SMALL = 64,
	MEDIUM = 128,
	LARGE = 256
};

void Preprocess(cv::Mat& input, cv::Mat& labels)
{
	for (int k = 1; k < 15; ++k)
	{
		for (int j = 0; j < 16; ++j)
			for (int i = 0; i < 16; ++i)
			{
				std::string filePath = "../Resources/" + std::to_string(64) + "-gray/notwaldo/" + std::to_string(k) + "_" + std::to_string(j) + "_" + std::to_string(i) + ".jpg";
				cv::Mat file;
				if (!FileExists(filePath))
				{
					filePath = "../Resources/" + std::to_string(64) + "-gray/waldo/" + std::to_string(k) + "_" + std::to_string(j) + "_" + std::to_string(i) + ".jpg";
					labels.push_back(1);
				}
				else
				{
					labels.push_back(-1);
				}
				file = cv::imread(filePath);
				file = file.reshape(1, 1);

				file.convertTo(file, CV_32F);
				input.push_back(file);

			}
	}

	return;
}

int main() {
	// Data for visual representation
	int width = 512, height = 512;

	ImgSize SIZE = ImgSize::SMALL;

	cv::Mat input, labels;// = cv::Mat(width, height, CV_32FC4);

	GenerateDataset(input, labels);

	Preprocess(input, labels);
	
	//input.convertTo(input, 16);
	cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();

	svm->setType(cv::ml::SVM::C_SVC);
	svm->setKernel(cv::ml::SVM::LINEAR);
	svm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, 100, 1e-6));
	
	cv::Ptr<cv::ml::TrainData> traindata = cv::ml::TrainData::create(input, cv::ml::ROW_SAMPLE, labels);

	svm->train(traindata);

	for (int j = 0; j < 16; ++j)
	for (int i = 0; i < 16; ++i)
	{
		std::string testfilePath = "../Resources/" + std::to_string(SIZE) + "-gray/notwaldo/12_" + std::to_string(j) + "_" + std::to_string(i) + ".jpg";
		cv::Mat file;
		if (!FileExists(testfilePath))
		{
			testfilePath = "../Resources/" + std::to_string(SIZE) + "-gray/waldo/12_" + std::to_string(j) + "_" + std::to_string(i) + ".jpg";
			std::cout << "TRUE    :     ";
		}
		else
			std::cout << "FALSE       :       ";

		cv::Mat test = cv::imread(testfilePath);
		test = test.reshape(1, 1);
		test.convertTo(test, CV_32F);

		float what = svm->predict(test);
		if (what == 1.0f)
			std::cout << "TRUE";
		else
			std::cout << "FALSE";
		std::cout << std::endl;
	}
	
	//cv::imshow("SVM Simple Example", input); // show it to the user
	
	cv::waitKey(0); 

	std::system("pause");
	
	return(0);

}

