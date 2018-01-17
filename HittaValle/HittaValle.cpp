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
#include <vector>
#include <limits.h>


void GenerateDataset(cv::Mat &dataset, cv::Mat &labels);

inline bool FileExists(std::string name)
{
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}

cv::Ptr<cv::ml::SVM> GetTrainedSVM(cv::Mat data, cv::Mat labels)
{
	cv::Ptr<cv::ml::SVM> model = cv::ml::SVM::create();

	model->setType(cv::ml::SVM::C_SVC);
	model->setKernel(cv::ml::SVM::LINEAR);
	model->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, 100, 1e-6));

	cv::Ptr<cv::ml::TrainData> traindata = cv::ml::TrainData::create(data, cv::ml::ROW_SAMPLE, labels);

	model->train(traindata);

	int stopper = 0;

	return model;
}

cv::Mat Preprocess(cv::Mat sample)
{
	cv::Mat processedSample;

	cv::cvtColor(sample, processedSample, CV_BGR2GRAY);
	cv::GaussianBlur(processedSample, processedSample, cv::Size(5, 5), 2.0f);


	processedSample = processedSample.reshape(1,1);
	processedSample.convertTo(processedSample, CV_32F);

	return processedSample;
}

bool PredictImage(cv::Ptr<cv::ml::StatModel> model, std::string sImagePath, int iDim)
{
	cv::Mat image = cv::imread(sImagePath);

	int iWidth = image.size().width;
	int iHeight = image.size().height;

	const int iStride = 8;
	cv::Mat subSample = cv::Mat(iDim, iDim, CV_8U); 

	std::vector<cv::Point> vMajorityVote;

	for (int x = 0; x < iWidth - iDim - iStride; x += iStride)
	{
		for (int y = 0; y < iHeight - iDim - iStride; y += iStride)
		{
			int iOffsX = x;
			int iOffsY = y;
			cv::Rect subRect = cv::Rect(iOffsX, iOffsY, iDim, iDim);

			image(subRect).copyTo(subSample);//this is so slow?...

			cv::imshow("Where is Wally?", Preprocess(subSample));//.reshape(1,1));
			
			if (model->predict(subSample) == 1.0f)//predicted sample to be wally
			{
				vMajorityVote.push_back(cv::Point(x, y));
			}
		}
	}
	std::cout << "predictions done" << std::endl;

	int iSmallestMagnitude = INT_MAX;
	int iMajorIndex = 0;
	for (int i = 0; i < vMajorityVote.size(); ++i)
	{
		int iMagComulator = 0;
		for (cv::Point p : vMajorityVote)
		{
			int iX = vMajorityVote[i].x - p.x;
			int iY = vMajorityVote[i].y - p.y;
			iMagComulator += iX * iX + iY * iY;
		}
		if (iMagComulator < iSmallestMagnitude)
		{
			iSmallestMagnitude = iMagComulator;
			iMajorIndex = i;
		}
	}

	cv::circle(image, cv::Point(vMajorityVote[iMajorIndex].x + iDim / 2, vMajorityVote[iMajorIndex].y + iDim / 2), iDim - iDim / 3, cv::Scalar(128, 128, 255), 5);
	
	cv::imshow("Where is Wally?", image);
	cv::waitKey(0);

	return true;
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

	int iDim = 128;
	
	const int iSize = iDim * iDim;//nr of pixels
	
	float* pAns = new float[iSize];
	float* pResult = new float[iSize];

	std::string sImagePath = "../Resources/original-images/8.jpg";
	
	cv::Mat data = cv::Mat();
	cv::Mat labels = cv::Mat();
	GenerateDataset(data, labels);
	std::cout << "dataset done" << std::endl;
	cv::Ptr<cv::ml::SVM> model = GetTrainedSVM(data, labels);
	std::cout << "Trained SVM" << std::endl;
	PredictImage(model, sImagePath, iDim);



	
	
	//std::system("pause");

	delete pAns;
	delete pResult;
	
	return(0);

}

