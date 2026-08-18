#include "CameraManager.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>

int main()
{
	int i[1]{ 0 };
	CameraManager test = CameraManager(i);
	test.StartCamera(0);
    return 0;
}