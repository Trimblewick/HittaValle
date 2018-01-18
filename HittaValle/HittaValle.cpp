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

void Heatmap(std::vector<cv::Point> predictions, cv::Mat &image)
{
	//cv::Mat heatmap = cv::Mat::zeros(image.size().height, image.size().width, CV_8U);
	int* heatmap = new int[image.size().height * image.size().width]();
	int max = 0;
	int maxX = 0;
	int maxY = 0;
	for (cv::Point prediction : predictions)
	{
		int x1 = std::max(prediction.x - 7, 0);
		int x2 = std::min(prediction.x + 8, image.size().width);

		int y1 = std::max(prediction.y - 7, 0);
		int y2 = std::min(prediction.y + 8, image.size().width);

		for (int y = y1; y < y2; ++y)
		{
			for (int x = x1; x < x2; ++x)
			{
				int pos = y * image.size().width + x;
				++heatmap[pos];
				if (max < heatmap[pos])
				{
					max = heatmap[pos];
					maxX = x;
					maxY = y;
				}
				/*cv::Vec3b intensity = heatmap.at<cv::Vec3b>(cv::Point(x, y));
				intensity.val[0] = uchar(intensity.val[0] + 1);
				heatmap.at<cv::Vec3b>(cv::Point(x, y)) = intensity;*/
			}
		}
	}

	cv::circle(image, cv::Point(maxX + 128 / 2, maxY + 128 / 2), 128 - 128 / 3, cv::Scalar(128, 128, 255), 10);

	return;
}

cv::Ptr<cv::ml::SVM> GetTrainedSVM(cv::Mat data, cv::Mat labels)
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

cv::Mat Preprocess(cv::Mat sample)
{
	cv::Mat processedSample;

	cv::cvtColor(sample, processedSample, CV_BGR2GRAY);
	cv::GaussianBlur(processedSample, processedSample, cv::Size(5, 5), 1.0f);


	processedSample = processedSample.reshape(1, 1);
	processedSample.convertTo(processedSample, CV_32F);

	return processedSample;
}


void GenerateDataset(cv::Mat &dataset, cv::Mat &labels)
{
	unsigned int size = 128, stride = 16;

	std::ifstream file;
	file.open("../Resources/ValleData.txt");
	if (!file.is_open())
	{
		return;
	}
	for(int i = 0; /*!file.eof()*/i < 2; ++i)
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

			for (int j = 0; j + size <= height; j += stride)
			{
				for (int k = 0; k + size <= width; k += stride)
				{
					cv::Mat partition(size, size, CV_8U);
					image(cv::Rect(k, j, size, size)).copyTo(partition);
					dataset.push_back(Preprocess(partition));
					/*
					cv::Mat testRed[3];
					cv::split(partition, testRed);
					cv::Mat holdRed = testRed[2].reshape(1, 1);
					holdRed.convertTo(holdRed, CV_32F);
					dataset.push_back(holdRed);*/

					if (x1 >= k && x2 <= k + size && y1 >= j && y2 <= j + size) //if Valle is within the partition
					{
						labels.push_back(1);
						/*labels.push_back(1);

						cv::flip(partition, partition, 1);//0 for flip x, 1 for flip y, -1 for flip both
						dataset.push_back(Preprocess(partition));
						labels.push_back(1);

						cv::split(partition, testRed);
	
						holdRed = testRed[2].reshape(1, 1);
						holdRed.convertTo(holdRed, CV_32F);
						dataset.push_back(holdRed);
						labels.push_back(1);*/
						

						stride = 4;
					}
					else
					{
						labels.push_back(-1);
						//labels.push_back(-1);
						stride = 16;
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

			image(subRect).copyTo(subSample);

			if (model->predict(Preprocess(subSample)) == 1)//predicted sample to be wally
			{
				vMajorityVote.push_back(cv::Point(x, y));
			}
		}
	}
	std::cout << "predictions done" << std::endl;

	std::cout << "nr of potential valles : " << std::to_string(vMajorityVote.size()) << std::endl;

	int iSmallestMagnitude = INT_MAX;
	
	/*int iMajorIndex = 0;
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
	}*/

	if (vMajorityVote.size() > 0)
	{
		//cv::circle(image, cv::Point(vMajorityVote[iMajorIndex].x + iDim / 2, vMajorityVote[iMajorIndex].y + iDim / 2), iDim - iDim / 3, cv::Scalar(128, 128, 255), 10);

		//cv::imwrite("../Resources/Imposter.jpg", image);

		//cv::Mat heatmap = Heatmap(vMajorityVote, image);
		Heatmap(vMajorityVote, image);
		cv::resize(image, image, cv::Size(image.size().width / 4.0f, image.size().height / 4.0f));// , 0.25, 0.25);
		//cv::resize(heatmap, heatmap, cv::Size(heatmap.size().width / 4.0f, heatmap.size().height / 4.0f));

		cv::imshow("Where is Wally?", image);
		//cv::imshow("Heatmap", heatmap);
		cv::waitKey(0);

		return true;
	}

	return false;
}

int main() {

	int iDim = 128;

	std::string sImagePath = "../Resources/3.jpg";

	cv::Mat data;
	cv::Mat labels;
	GenerateDataset(data, labels);
	std::cout << "dataset done" << std::endl;
	cv::Ptr<cv::ml::SVM> model = GetTrainedSVM(data, labels);
	std::cout << "Trained SVM" << std::endl;
	PredictImage(model, sImagePath, iDim);

	return(0);

}