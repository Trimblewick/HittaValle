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

inline bool FileExists(std::string name)
{
	struct stat buffer;
	return (stat(name.c_str(), &buffer) == 0);
}


/*
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
	}

	return;
}
*/
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

cv::Mat Preprocess(cv::Mat sample)
{
	cv::Mat processedSample;

	processedSample = sample.reshape(1,1);
	processedSample.convertTo(sample, CV_32F);

	return processedSample;
}

bool PredictImage(/*cv::Ptr<cv::ml::StatModel> model, */std::string sImagePath, int iDim)
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
			
			//if (model->predict(subSample) == -1.0f)//not wally
			if ((x == 16 && y == 16) || (x == iStride * 100 && y == iStride * 20) || (x == iStride * 105 && y == iStride * 22))
			{
				vMajorityVote.push_back(cv::Point(x, y));

			}
			//else//is wally
			{
				
			}

		}
	}

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
	
	cv::Mat* pInput = new cv::Mat();
	cv::Mat* pLabels = new cv::Mat();

	PredictImage(sImagePath, iDim);



	
	
	//std::system("pause");

	delete pAns;
	delete pResult;
	delete pInput;
	delete pLabels;

	return(0);

}

