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


void TrainSVM(int testImageIndex, std::vector<cv::Mat> dataset, std::vector<cv::Mat> labels, cv::Ptr<cv::ml::SVM> &model)
{
	cv::Mat trainingData = cv::Mat();
	cv::Mat trainingLabels = cv::Mat();
	for (int i = 0; i < dataset.size(); ++i)
	{
		if (i != testImageIndex)
		{
			trainingData.push_back(dataset[i]);
			trainingLabels.push_back(labels[i]);
		}
	}
	model = GetTrainedSVM(trainingData, trainingLabels);
}
void GenerateDataset(std::vector<cv::Mat> &dataset, std::vector<cv::Mat> &labels)
{
	unsigned int size = 128, stride = 16;

	std::ifstream file;
	file.open("../Resources/ValleData.txt");
	if (!file.is_open())
		return;

	for(int i = 0; !file.eof(); ++i)
	{
		dataset.push_back(cv::Mat());
		labels.push_back(cv::Mat());
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
					dataset[0].push_back(Preprocess(partition));

					if (x1 >= k && x2 <= k + size && y1 >= j && y2 <= j + size) //if Valle is within the partition
					{
						labels[0].push_back(1);


						stride = 4;
					}
					else
					{
						labels[0].push_back(-1);
						stride = 16;
					}
				}
			}
		}
	}
	file.close();

	std::reverse(dataset.begin(), dataset.end());
	std::reverse(labels.begin(), labels.end());
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

					if (x1 >= k && x2 <= k + size && y1 >= j && y2 <= j + size) //if Valle is within the partition
					{
						labels.push_back(1);

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

	if (vMajorityVote.size() > 0)
	{
		Heatmap(vMajorityVote, image);
		
		cv::resize(image, image, cv::Size(image.size().width / 4.0f, image.size().height / 4.0f));

		cv::imshow("Where is Wally?", image);
		cv::waitKey(0);

		return true;
	}

	return false;
}

int main() {

	int iDim = 128;

	std::string sImagePath = "../Resources/3.jpg";

	std::vector<cv::Mat> data;
	std::vector<cv::Mat> labels;
	GenerateDataset(data, labels);
	std::cout << "dataset done" << std::endl;
	cv::Ptr<cv::ml::SVM> model;
	for (int i = 0; i < data.size(); ++i)
	{
		TrainSVM(i, data, labels, model);

		std::cout << "Trained SVM" << std::endl;
		//PredictImage(model, sImagePath, iDim);
		//Kvar är att göra predictions och räkna fmeasure ist för den prediction som görs nu.
	}

	return(0);

}