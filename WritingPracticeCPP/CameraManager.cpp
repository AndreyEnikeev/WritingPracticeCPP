#include "CameraManager.h"
#include <opencv2/opencv.hpp>
#include <iostream>

void CameraManager::StartCameras_All()
{
	for (int i = 0; i < openCamers; i++)
	{
		workFlag = true;
		StartCamera(i);
		cv::namedWindow("Test" + std::to_string(i), cv::WindowFlags::WINDOW_FULLSCREEN);
	}
	while (workFlag)
	{
		cv::imshow("Test", clearFrame);
		if (cv::waitKey(1) == 32)
		{
			workFlag = false;
		}
	}
	StopCameras_All();
	cv::destroyAllWindows();
}

void CameraManager::StartCamera(int index)
{
	if (index < openCamers && cameras[index].isOpened())
	{
		threadsCameras.emplace_back((readFrame, std::ref(cameras[index])));

	}
}


void CameraManager::readFrame(cv::VideoCapture& camera)
{
	cv::Mat clearFrame;
	while (workFlag)
	{
		if (camera.read(clearFrame))
		{
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