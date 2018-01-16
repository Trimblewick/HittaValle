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


#define NR_WALLY_IMAGES 15


void Preprocess(cv::Mat*& pInput, cv::Mat*& pLabels, int iDim)
{
	/*for (int k = 1; k < NR_WALLY_IMAGES; ++k)
	{
		for (int j = 0; j < iDim; ++j)
			for (int i = 0; i < iDim; ++i)
			{
				std::string filePath = "../Resources/" + std::to_string(ImgSize::MEDIUM) + "/notwaldo/" + std::to_string(k) + "_" + std::to_string(j) + "_" + std::to_string(i) + ".jpg";
				cv::Mat file;
				if (!FileExists(filePath))
				{
					filePath = "../Resources/" + std::to_string(ImgSize::MEDIUM) + "/waldo/" + std::to_string(k) + "_" + std::to_string(j) + "_" + std::to_string(i) + ".jpg";
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
	}*/

	return;
}

cv::Ptr<cv::ml::SVM> GetTrainedSVM(cv::Mat* pInput, cv::Mat* pLabels)
{
	cv::Ptr<cv::ml::SVM> model = cv::ml::SVM::create();

	model->setType(cv::ml::SVM::C_SVC);
	model->setKernel(cv::ml::SVM::LINEAR);
	model->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, 100, 1e-6));

	cv::Ptr<cv::ml::TrainData> traindata = cv::ml::TrainData::create(*pInput, cv::ml::ROW_SAMPLE, *pLabels);

	model->train(traindata);

	return model;
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

	int iDim = 64;
	const int iSize = iDim * iDim;
	float* pAns = new float[iSize];
	float* pResult = new float[iSize];

	std::string sImagePath = "../Resources/original-images/8.jpg";
	cv::Mat image = cv::imread(sImagePath);
	cv::Mat* pInput = new cv::Mat();
	cv::Mat* pLabels = new cv::Mat();

	int iOffsX = 1150;
	int iOffsY = 290;
	
	cv::Rect subRect = cv::Rect(iOffsX, iOffsY, iDim, iDim);
	*pInput = image(subRect);

	//Preprocess(pInput, pLabels, iDim);

	//cv::Ptr<cv::ml::StatModel> model = GetTrainedSVM(pInput, pLabels);
	/*
	//Generate answers for testset, and make predictions
	for (int j = 0; j < iDim; ++j)
		for (int i = 0; i < iDim; ++i)
		{
			int index = j * iDim + i;
			std::string testfilePath = "../Resources/" + std::to_string(SIZE) + "/notwaldo/18_" + std::to_string(j) + "_" + std::to_string(i) + ".jpg";
			cv::Mat file;

			//Velle or not?
			if (!FileExists(testfilePath))
			{
				testfilePath = "../Resources/" + std::to_string(SIZE) + "/waldo/18_" + std::to_string(j) + "_" + std::to_string(i) + ".jpg";
				pAns[index] = 1.0f;
			}
			else
				pAns[index] = 0.0f;
		
			//predict results
			cv::Mat test = cv::imread(testfilePath);
			test = test.reshape(1, 1);
			test.convertTo(test, CV_32F);

			pResult[index] = model->predict(test);		
		}
	*/
	
	//Valledate(pAns, pResult, iSize);
	
	cv::imshow("SVM Simple Example", *pInput); // show it to the user

	cv::waitKey(0);
	
	//std::system("pause");

	delete pAns;
	delete pResult;
	delete pInput;
	delete pLabels;

	return(0);

}

