#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2\ml.hpp>
#include <opencv2\ml\ml.hpp>

#include <iostream>
#include <conio.h>           // may have to modify this line if not using Windows
#include <vector>
#include <limits.h>

//inline bool FileExists(std::string name)
//{
//	struct stat buffer;
//	return (stat(name.c_str(), &buffer) == 0);
//}

cv::Ptr<cv::ml::SVM> CreateTrainedSVM(cv::Mat data, cv::Mat labels)
{
	cv::Ptr<cv::ml::SVM> model = cv::ml::SVM::create();

	model->setType(cv::ml::SVM::C_SVC);
	model->setKernel(cv::ml::SVM::LINEAR);
	model->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, 100, 1e-6));

	cv::Ptr<cv::ml::TrainData> traindata = cv::ml::TrainData::create(data, cv::ml::ROW_SAMPLE, labels);

	std::cout << "started training" << std::endl;
	
	model->train(traindata);

	return model;
}


cv::Ptr<cv::ml::SVM> LoadSVM(std::string filePath)
{
	cv::Ptr<cv::ml::SVM> model = cv::ml::SVM::load(filePath);//create();
	
	//model->load(filePath);
	
	return model;
}

cv::Mat Preprocess(cv::Mat sample)
{
	cv::Mat processedSample;

	cv::cvtColor(sample, processedSample, CV_BGR2GRAY);
	cv::GaussianBlur(processedSample, processedSample, cv::Size(5, 5), 1.0f);


	processedSample = processedSample.reshape(1, 1);
	processedSample.convertTo(processedSample, CV_32F);

	return processedSample;
}

cv::Mat PreprocessRedChannel(cv::Mat sample)
{
	cv::Mat processedSample;
	cv::Mat bgr[3];

	cv::split(sample, bgr);

	processedSample = bgr[2];

	processedSample = processedSample.reshape(1, 1);
	processedSample.convertTo(processedSample, CV_32F);

	return processedSample;
}

cv::Mat PreprocessRedChannelCanny(cv::Mat sample)
{
	cv::Mat processedSample;
	cv::Mat bgr[3];
	
	cv::split(sample, bgr);

	processedSample = bgr[2];
	

	cv::GaussianBlur(processedSample, processedSample, cv::Size(7, 7), 2.0f);
	cv::Canny(processedSample, processedSample, 64, 128);


	processedSample = processedSample.reshape(1, 1);
	processedSample.convertTo(processedSample, CV_32F);
	
	return processedSample;
}



void GenerateDataset(cv::Mat &dataset, cv::Mat &labels, int iDim)
{
	unsigned int stride = 16;

	std::ifstream file;
	file.open("../Resources/ValleTest.txt");
	if (!file.is_open())
	{
		return;
	}
	for(int i = 0; !file.eof(); ++i)
	{
		std::cout << std::to_string(i) << std::endl;
		cv::Mat image = cv::imread("../Resources/" + std::to_string(i) + ".jpg");
		

		if (image.size().area())
		{
			int width = image.size().width;
			int height = image.size().height;


			int x, x1, x2, y, y1, y2;

			file >> x;
			x1 = std::max(x - 31, 0);
			x2 = std::min(x + 32, width);

			file >> y;
			y1 = std::max(y - 31, 0);
			y2 = std::min(y + 32, height);

			for (int j = 0; j + iDim <= height; j += stride)
			{
				for (int k = 0; k + iDim <= width; k += stride)
				{
					
					cv::Mat partition(iDim, iDim, CV_8U);
					image(cv::Rect(k, j, iDim, iDim)).copyTo(partition);

					dataset.push_back(Preprocess(partition));
					dataset.push_back(PreprocessRedChannel(partition));
					dataset.push_back(PreprocessRedChannelCanny(partition));
					cv::flip(partition, partition, 1);//0 for flip x, 1 for flip y, -1 for flip both
					dataset.push_back(Preprocess(partition));
					dataset.push_back(PreprocessRedChannel(partition));
					dataset.push_back(PreprocessRedChannelCanny(partition));

					if (x1 >= k && x2 <= k + iDim && y1 >= j && y2 <= j + iDim) //if Valle is within the partition
					{
						
						labels.push_back(1);
						labels.push_back(1);
						labels.push_back(1);
						labels.push_back(1);
						labels.push_back(1);
						labels.push_back(1);
					}
					else
					{
						labels.push_back(-1);
						labels.push_back(-1);
						labels.push_back(-1);
						labels.push_back(-1);
						labels.push_back(-1);
						labels.push_back(-1);
					}
				}
			}
		}
	}
	file.close();
}


bool PredictImage(cv::Ptr<cv::ml::StatModel> model, std::string sImagePath, int iDim)
{
	cv::Mat image = cv::imread(sImagePath);

	int iWidth = image.size().width;
	int iHeight = image.size().height;

	const int iStride = 16;
	cv::Mat subSample = cv::Mat(iDim, iDim, CV_8U);

	std::vector<cv::Point> vMajorityVote;

	for (int x = 0; x < iWidth - iDim - iStride; x += iStride)
	{
		for (int y = 0; y < iHeight - iDim - iStride; y += iStride)
		{
			int iOffsX = x;
			int iOffsY = y;
			cv::Rect subRect = cv::Rect(iOffsX, iOffsY, iDim, iDim);
			int iNumberOfPredictions = 0;
			
			image(subRect).copyTo(subSample);

			if (model->predict(Preprocess(subSample)) == 1)
				iNumberOfPredictions++;
			if (model->predict(PreprocessRedChannel(subSample)) == 1)
				iNumberOfPredictions++;
			if (model->predict(PreprocessRedChannelCanny(subSample)) == 1)
				iNumberOfPredictions++;

			for (int i = 0; i < iNumberOfPredictions; ++i)
				vMajorityVote.push_back(cv::Point(x, y));
			
		}
	}
	std::cout << "predictions done" << std::endl;

	std::cout << "nr of potential valles : " << std::to_string(vMajorityVote.size()) << std::endl;

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

	if (vMajorityVote.size() > 0)
	{
		cv::circle(image, cv::Point(vMajorityVote[iMajorIndex].x + iDim / 2, vMajorityVote[iMajorIndex].y + iDim / 2), iDim, cv::Scalar(64, 64, 255), 10);

		//cv::imwrite("../Resources/Imposter.jpg", image);

		cv::resize(image, image, cv::Size(image.size().width / 2.0f, image.size().height / 2.0f));// , 0.25, 0.25);

	cv::imshow("Where is Wally?", image);
	cv::waitKey(0);

	return true;
}

	return false;
}

int main() {

	int iDim = 128;

	std::string sImagePath = "../Resources/0.jpg";

	cv::Mat data;
	cv::Mat labels;
	GenerateDataset(data, labels, iDim);
	std::cout << "dataset done" << std::endl;
	cv::Ptr<cv::ml::SVM> model = CreateTrainedSVM(data, labels);
	//cv::Ptr<cv::ml::SVM> model = LoadSVM("my_own_svm.xml");//
	model->save("my_own_svm.xml");

	std::cout << "trained SVM" << std::endl;



	PredictImage(model, sImagePath, iDim);

	return(0);

}