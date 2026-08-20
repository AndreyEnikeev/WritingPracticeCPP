#include "CameraManager.h"
#include <opencv2/opencv.hpp>
#include <iostream>

void CameraManager::CreatCameras(int index[], size_t countIndex)
{
	for (size_t i = 0; i < countIndex; ++i)
	{
		cv::VideoCapture cap;
		if (cap.open(index[i], cv::CAP_DSHOW))
		{
			SetParametersCameraStandard(cap);
			cameras.push_back(std::move(cap));
			openCamers++;
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
		else
		{
			std::cerr << "Fail start " << index[i] << std::endl;
		}
	}
	if (openCamers > 0)
	{
		SetParametersCameraScreen(cameras.back());
		lastFrames.resize(openCamers);
		frameMutexes.resize(openCamers);
	}
}
void CameraManager::StartCameras_ToCheckWork()
{
	if (openCamers > 0)
	{
		workFlag = true;
		for (int i = 0; i < openCamers; i++)
		{
			StartThreadReadFrame(i);
			cv::namedWindow("Test" + std::to_string(i), cv::WindowFlags::WINDOW_FULLSCREEN);
		}
		std::vector<cv::Mat> showFrames(openCamers);
		while (workFlag)
		{
			for (int i = 0; i < openCamers; i++)
			{
				{
					std::lock_guard<std::mutex> lock(frameMutexes[i]);
					if (!lastFrames[i].empty())
					{
						lastFrames[i].copyTo(showFrames[i]);
					}
				}
				if (!showFrames[i].empty())
				{
					cv::imshow("Test" + std::to_string(i), showFrames[i]);
				}
			}
			if (cv::waitKey(1) == 32)
			{
				workFlag = false;
			}
		}
		for (cv::Mat& frame : showFrames)
		{
			frame.release();
		}
	}
	StopCameras_All();
	cv::destroyAllWindows();
}

void CameraManager::StartThreadReadFrame(int index)
{
	if (index < openCamers && cameras[index].isOpened())
	{
		threadsCameras.emplace_back(&CameraManager::ReadFrame, this, index);
	}
}

void CameraManager::ReadFrame(int index)
{
	cv::Mat clearFrame;
	while (workFlag)
	{
		if (cameras[index].read(clearFrame))
		{
			std::lock_guard<std::mutex> lock(frameMutexes[index]);
			clearFrame.copyTo(lastFrames[index]);
		}
	}
	clearFrame.release();
}
void CameraManager::StopCameras_All()
{
	for (auto& th : threadsCameras) {
		if (th.joinable()) {
			th.join();
		}
	}
	threadsCameras.clear();
}

void CameraManager::SetParametersCameraStandard(cv::VideoCapture& camera) const
{
	camera.set(cv::CAP_PROP_FRAME_WIDTH, WidthFrames);
	camera.set(cv::CAP_PROP_FRAME_HEIGHT, HeightFrames);
	camera.set(cv::CAP_PROP_BITRATE, bitrate);
	camera.set(cv::CAP_PROP_FPS, FPSStandard);
}
void CameraManager::SetParametersCameraScreen(cv::VideoCapture& camera) const
{
	camera.set(cv::CAP_PROP_FRAME_WIDTH, WidthFrames);
	camera.set(cv::CAP_PROP_FRAME_HEIGHT, HeightFrames);
	camera.set(cv::CAP_PROP_BITRATE, bitrate);
	camera.set(cv::CAP_PROP_FPS, FPSScreenCamera);
}