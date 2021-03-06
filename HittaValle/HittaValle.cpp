#include "stdafx.h"

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2\ml.hpp>
#include <opencv2\ml\ml.hpp>
#include <opencv2\features2d\features2d.hpp>

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
	cv::Mat heatmapc = cv::Mat::zeros(image.size().height, image.size().width, CV_8U);
	int* heatmap = new int[image.size().height * image.size().width]();

	int max = 0;
	int maxX = 0;
	int maxY = 0;
	for (cv::Point prediction : predictions)
	{
		int x1 = std::max(prediction.x - 7, 0);
		int x2 = std::min(prediction.x + 8, image.size().width);

		int y1 = std::max(prediction.y - 7, 0);
		int y2 = std::min(prediction.y + 8, image.size().height);

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
		cv::circle(image, cv::Point(prediction.x + 128 / 2, prediction.y + 128 / 2), 20, cv::Scalar(128, 128, 255, 64), 10);
	}

	

	return;

}

cv::Ptr<cv::ml::SVM> CreateTrainedSVM(cv::Mat data, cv::Mat labels)
{
	cv::Ptr<cv::ml::SVM> model = cv::ml::SVM::create();

	model->setType(cv::ml::SVM::C_SVC);
	model->setKernel(cv::ml::SVM::LINEAR);
	model->setTermCriteria(cv::TermCriteria(cv::TermCriteria::MAX_ITER, 1000, 1e-6));

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

/*
	processedSample = processedSample.reshape(1, 1);
	processedSample.convertTo(processedSample, CV_32F);*/

	return processedSample;
}

cv::Mat PreprocessRedChannel(cv::Mat sample)
{
	cv::Mat processedSample;
	cv::Mat bgr[3];

	cv::split(sample, bgr);

	processedSample = bgr[2];

	//processedSample = processedSample.reshape(1, 1);
	//processedSample.convertTo(processedSample, CV_32F);

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
	cv::GaussianBlur(processedSample, processedSample, cv::Size(3, 3), 2.5f);

	//processedSample = processedSample.reshape(1, 1);
	//processedSample.convertTo(processedSample, CV_32F);
	
	return processedSample;
}

bool PreprocessExtractFeatures(cv::Mat sample, cv::Mat& processedSample, int iV)
{
	
	cv::Mat r = PreprocessRedChannel(sample);


	std::vector<cv::KeyPoint> keypoints;

	std::vector<cv::Point2f> pp;
	std::vector<int> ip;
	cv::FAST(r, keypoints, 35);
	
	
	for (cv::KeyPoint p : keypoints)
	{
		pp.push_back(p.pt);
		ip.push_back(iV);
	}

	if (keypoints.size() < 64)
	{
		return false;
	}
	
	cv::kmeans(pp, 64, ip, cv::TermCriteria(cv::TermCriteria::Type::MAX_ITER, 1500, 1e-6), 5, cv::KmeansFlags::KMEANS_RANDOM_CENTERS, processedSample);

	processedSample = processedSample.reshape(1, 1);
	processedSample.convertTo(processedSample, CV_32F);

	return true;
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
	model = CreateTrainedSVM(trainingData, trainingLabels);
}
void GenerateDataset(std::vector<cv::Mat> &dataset, std::vector<cv::Mat> &labels, int iDim)
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
		cv::Mat image = cv::imread("../Resources/" + std::to_string(i) + std::to_string(i) + ".png");

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


					int iValleLabel = -1;

					if (x1 >= k && x2 <= k + iDim && y1 >= j && y2 <= j + iDim) //if Valle is within the partition
					{
						stride = 10;
						iValleLabel = 1;

					}
					else
						stride = 10;


					cv::Mat processedSample = cv::Mat();
					if (PreprocessExtractFeatures(partition, processedSample, iValleLabel))
					{
						dataset[0].push_back(processedSample);
						labels[0].push_back(iValleLabel);
						cv::flip(partition, partition, 1);
						if (PreprocessExtractFeatures(partition, processedSample, iValleLabel))
						{
							dataset[0].push_back(processedSample);
							labels[0].push_back(iValleLabel);
						}
					}


					//dataset[0].push_back(Preprocess(partition));

					//if (x1 >= k && x2 <= k && y1 >= j && y2 <= j) //if Valle is within the partition
					//{
					//	labels[0].push_back(1);


					//	stride = 4;
					//}
					//else
					//{
					//	labels[0].push_back(-1);
					//	stride = 16;
					//}
				}
			}
		}
	}
	file.close();

	std::reverse(dataset.begin(), dataset.end());
	std::reverse(labels.begin(), labels.end());
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
		std::cout << "startedTraining" << std::to_string(i) << std::endl;
		cv::Mat image = cv::imread("../Resources/" + std::to_string(i) + std::to_string(i) + ".png");
		

		if (image.size().area())
		{
			int width = image.size().width;
			int height = image.size().height;


			int x, x1, x2, y, y1, y2;

			file >> x;
			x1 = std::max(x - 9, 0);
			x2 = std::min(x + 10, width);

			file >> y;
			y1 = std::max(y - 19, 0);
			y2 = std::min(y + 1, height);

			for (int j = 0; j + iDim <= height; j += stride)
			{
				for (int k = 0; k + iDim <= width; k += stride)
				{/*
					cv::kmeans()

					cv::BOWKMeansTrainer bow(64, cv::TermCriteria(CV_TERMCRIT_ITER, 10, 0.001), 1);*/
					cv::Mat partition(iDim, iDim, CV_8U);
					image(cv::Rect(k, j, iDim, iDim)).copyTo(partition);

					//cv::Mat rc = PreprocessRedChannelCanny(partition);


					int iValleLabel = -1;

					if (x1 >= k && x2 <= k + iDim && y1 >= j && y2 <= j + iDim) //if Valle is within the partition
					{
						stride = 10;
						iValleLabel = 1;
						//							labels.push_back(1);



						//cv::imshow("asdf", partition);
						//cv::waitKey(0);

					}
					else
						stride = 10;
						
					
					cv::Mat processedSample = cv::Mat();
					if (PreprocessExtractFeatures(partition, processedSample, iValleLabel))
					{
						dataset.push_back(processedSample);
						labels.push_back(iValleLabel);
						cv::flip(partition, partition, 1);
						if (PreprocessExtractFeatures(partition, processedSample, iValleLabel))
						{
							dataset.push_back(processedSample);
							labels.push_back(iValleLabel);
						}
					}
				}

			}
			
			int stopper = 0;
		}
	}
	file.close();
}

void Fmeasure(cv::Ptr<cv::ml::StatModel> model, std::string sImagePath, int iDim, int ValleX, int ValleY)
{
	cv::Mat image = cv::imread(sImagePath);

	int iWidth = image.size().width;
	int iHeight = image.size().height;

	const int iStride = 8;
	cv::Mat subSample = cv::Mat(iDim, iDim, CV_8U);

	int results[2][2] = { 0 };


	int x1 = std::max(ValleX - 31, 0);
	int x2 = std::min(ValleX + 32, iWidth);

	int y1 = std::max(ValleY - 31, 0);
	int y2 = std::min(ValleY + 32, iHeight);

	for (int x = 0; x < iWidth - iDim - iStride; x += iStride)
	{
		for (int y = 0; y < iHeight - iDim - iStride; y += iStride)
		{
			int iOffsX = x;
			int iOffsY = y;
			cv::Rect subRect = cv::Rect(iOffsX, iOffsY, iDim, iDim);

			image(subRect).copyTo(subSample);

			int prediction = model->predict(Preprocess(subSample));

			int actual = (x1 >= x && x2 <= x && y1 >= y && y2 <= y);
			
			++results[prediction][actual];

		}
	}
	float precision = (float)results[1][1] / ((float)results[1][1] + (float)results[1][0]);
	float recall = (float)results[1][1] / ((float)results[1][1] + (float)results[0][1]);
	float fmeasure = 2 * (precision * recall) / (precision + recall);

	std::ofstream file;
	file.open("../Results/data.csv");

	if (file.is_open())
	{
		file << results[1][1] << "," << results[0][1] << std::endl;
		file << results[1][0] << "," << results[0][0] << std::endl;
		file << std::endl << std::endl;
	}
}

bool PredictImage(cv::Ptr<cv::ml::StatModel> model, std::string sImagePath, int iDim)
{
	std::cout << "predicting..." << std::endl;
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
			int iNumberOfPredictions = 0;
			
			image(subRect).copyTo(subSample);

			cv::Mat processedSubSample = cv::Mat();

			if (PreprocessExtractFeatures(subSample, processedSubSample, 1))
			{
				if (model->predict(processedSubSample) == 1)
					iNumberOfPredictions++;
			}

			std::cout << iNumberOfPredictions << std::endl;

			for (int i = 0; i < iNumberOfPredictions; ++i)
			{
				//cv::circle(image, cv::Point(x + 128 / 2, y + 128 / 2), 128 - 128 / 3, cv::Scalar(128, 128, 255), 10);
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
		//cv::resize(image, image, cv::Size(image.size().width / 4.0f, image.size().height / 4.0f));// , 0.25, 0.25);
		//cv::resize(heatmap, heatmap, cv::Size(heatmap.size().width / 4.0f, heatmap.size().height / 4.0f));

		cv::imshow("Where is Wally?", image);
		cv::waitKey(0);

		

		return true;
	}
	
	return false;
}

int main() {

	int iDim = 64;
	//DownSampleSet();
	std::string sImagePath = "../Resources/00.png";
	
	std::vector<cv::Mat> data;
	std::vector<cv::Mat> labels;

	//GenerateDataset(data, labels, iDim);
	/*cv::Ptr<cv::ml::SVM> model = CreateTrainedSVM(data, labels);
	model->save("my_own_svm2.xml");*/
	cv::Ptr<cv::ml::SVM> model = LoadSVM("my_own_svm2.xml");//
	PredictImage(model, sImagePath, iDim);
	/*std::ifstream file;
	file.open("../Resources/ValleTest.txt");
	for (int i = 0; i < data.size(); ++i)
	{
		TrainSVM(i, data, labels, model);
		std::cout << "Done training SVM " << std::to_string(i) << std::endl;

		int x, y;
		file >> x;
		file >> y;
		Fmeasure(model, "../Resources/" + std::to_string(i) + std::to_string(i) + ".png", iDim, x, y);
		std::cout << "Fmeasure calculated for test set " << std::to_string(i) << std::endl;
	}*/
	

	return(0);

}