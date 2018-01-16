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

#define NR_WALLY_IMAGES 15


void Preprocess(cv::Mat*& pInput, cv::Mat*& pLabels, int iDim)
{
	for (int k = 1; k < NR_WALLY_IMAGES; ++k)
	{
		for (int j = 0; j < iDim; ++j)
			for (int i = 0; i < iDim; ++i)
			{
				std::string filePath = "../Resources/" + std::to_string(64) + "-gray/notwaldo/" + std::to_string(k) + "_" + std::to_string(j) + "_" + std::to_string(i) + ".jpg";
				cv::Mat file;
				if (!FileExists(filePath))
				{
					filePath = "../Resources/" + std::to_string(64) + "-gray/waldo/" + std::to_string(k) + "_" + std::to_string(j) + "_" + std::to_string(i) + ".jpg";
					pLabels->push_back(1);
				}
				else
				{
					pLabels->push_back(-1);
				}
				file = cv::imread(filePath);
				file = file.reshape(1, 1);

				file.convertTo(file, CV_32F);
				pInput->push_back(file);
			}
	}

	return;
}

void Train()
{


	return;
}

void Test()
{


	return;
}

void Valledate(float* pAns, float* pResult, int iSize)
{
	int iTruePositives = 0;
	int iTrueNegatives = 0;
	int iFalsePositives = 0;
	int iFalseNegatives = 0;

	for (int i = 0; i < iSize; ++i)
	{
		int iAns = pAns[i];
		int iRes = pResult[i];
		if (iAns == 0.0f)
		{
			if (iRes == 0.0f)
				++iTrueNegatives;
			else
				++iFalseNegatives;
		}
		else
		{
			if (iRes == 1.0f)
				++iTruePositives;
			else
				++iFalsePositives;
		}
	}

	std::cout << "True Positives : " + std::to_string(iTruePositives) << std::endl <<
		"True Negatives : " + std::to_string(iTrueNegatives) << std::endl <<
		"False Positives : " + std::to_string(iFalsePositives) << std::endl <<
		"False Negatives : " + std::to_string(iFalseNegatives) << std::endl;

	return;
}

int main() {

	ImgSize SIZE = ImgSize::SMALL;

	int iDim = 16;
	
	if (SIZE == ImgSize::MEDIUM)
		iDim = 8;
	else if (SIZE == ImgSize::LARGE)
		iDim = 4;

	cv::Mat* pInput = new cv::Mat();
	cv::Mat* pLabels = new cv::Mat();

	Preprocess(pInput, pLabels, iDim);
	

	cv::Ptr<cv::ml::SVM> svm = cv::ml::SVM::create();

	svm->setType(cv::ml::SVM::C_SVC);
	svm->setKernel(cv::ml::SVM::LINEAR);
	svm->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, 100, 1e-6));
	
	cv::Ptr<cv::ml::TrainData> traindata = cv::ml::TrainData::create(*pInput, cv::ml::ROW_SAMPLE, *pLabels);

	svm->train(traindata);

	const int iSize = iDim * iDim;
	float* pAns = new float[iSize];
	float* pResult = new float[iSize];

	for (int j = 0; j < iDim; ++j)
		for (int i = 0; i < iDim; ++i)
		{
			int index = j * iDim + i;
			std::string testfilePath = "../Resources/" + std::to_string(SIZE) + "-gray/notwaldo/12_" + std::to_string(j) + "_" + std::to_string(i) + ".jpg";
			cv::Mat file;
			if (!FileExists(testfilePath))
			{
				testfilePath = "../Resources/" + std::to_string(SIZE) + "-gray/waldo/12_" + std::to_string(j) + "_" + std::to_string(i) + ".jpg";
				pAns[index] = 1.0f;
			}
			else
				pAns[index] = 0.0f;

			cv::Mat test = cv::imread(testfilePath);
			test = test.reshape(1, 1);
			test.convertTo(test, CV_32F);

			pResult[index] = svm->predict(test);		
		}
	
	Valledate(pAns, pResult, iSize);

	//cv::imshow("SVM Simple Example", pInput); // show it to the user
	//cv::waitKey(0); 

	std::system("pause");

	delete pAns;
	delete pResult;
	delete pInput;
	delete pLabels;

	return(0);

}

