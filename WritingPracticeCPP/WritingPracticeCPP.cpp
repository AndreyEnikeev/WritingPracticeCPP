#include "CameraManager.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>

int main()
{
	int indices[] { 2, 1, 0};
	CameraManager test(indices, sizeof(indices) / sizeof(indices[0]));
	test.StartCameras_ToCheckWork();
    return 0;
}