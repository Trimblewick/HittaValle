#pragma once
#include "stdafx.h"
#include <fstream>
#include <sstream>
#include <algorithm>

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/opencv.hpp>
#include <opencv2\ml.hpp>
#include <opencv2\ml\ml.hpp>

void GenerateDataset(cv::Mat &dataset, cv::Mat &labels)
{
	unsigned int size = 128, stride = 16;

	std::ifstream file;
	file.open("../Resources/ValleData.txt");
	for(int i = 0; file.is_open(); ++i)
	{
		cv::Mat image = cv::imread("../Resources/" + std::to_string(i) + ".jpg");
		int width = image.size().width;
		int height = image.size().height;

		int x, x1, x2, y, y1, y2;

		file >> x;
		x1 = std::max(x - 31, 0);
		x2 = std::min(x + 32, width);

		file >> y;
		y1 = std::max(y - 31, 0);
		y2 = std::max(y + 32, height);

		for (int j = 0; j < height; j += stride)
		{
			for (int k = 0; k < width; k += stride)
			{
				cv::Mat partition(size, size, CV_8U);
				image(cv::Rect(k, j, k + size, j + size)).copyTo(partition);
				dataset.push_back(partition);

				float label = -1.f;

				if (x1 >= k && x2 <= k + size && y1 >= j && y2 <= j + size) //if Valle is within the partition
				{
					++++label;
				}
			}
		}
	}
}