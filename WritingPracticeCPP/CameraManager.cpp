#include "CameraManager.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <chrono>

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
		timesFrame.resize(openCamers);
		hasNewFrame.resize(openCamers);
	}
}
void CameraManager::StartCameras_ToCheckWork()
{
	if (openCamers > 0)
	{
		workFlag = true;
		startTime = std::chrono::steady_clock::now();
		for (int i = 0; i < openCamers; i++)
		{
			StartThreadReadFrame(i);
			cv::namedWindow("Test" + std::to_string(i), cv::WINDOW_FULLSCREEN);
		}
		std::vector<cv::Mat> showFrames(openCamers);
		while (workFlag)
		{
			for (int i = 0; i < openCamers; i++)
			{
				Duration time;
				if (getFrameWithReadyFlag(i, showFrames[i], &time))
				{
					cv::putText(showFrames[i], timeToString(time), cv::Point(0, 50), cv::FONT_HERSHEY_SIMPLEX, 2, cv::Scalar(0, 255, 0));
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

bool CameraManager::getScreenFrame(cv::Mat& out, Duration* durationSinceStart) const
{
	if (openCamers == 0) return false;
	return getFrame(openCamers - 1, out, durationSinceStart);
}
bool CameraManager::getScreenFrameWithReadyFlag(cv::Mat& out, Duration* durationSinceStart) const
{
	if (openCamers == 0) return false;
	return getFrameWithReadyFlag(openCamers - 1, out, durationSinceStart);
}
bool CameraManager::getFrame(size_t index, cv::Mat& out, Duration* durationSinceStart) const
{
	if (index >= openCamers)
	{
		return false;
	}
	std::lock_guard<std::mutex> lock(frameMutexes[index]);
	if (lastFrames[index].empty())
	{
		return false;
	}
	lastFrames[index].copyTo(out);
	if (durationSinceStart) *durationSinceStart = timesFrame[index] - startTime;
	return true;
}
bool CameraManager::getFrameWithReadyFlag(size_t index, cv::Mat& out, Duration* durationSinceStart) const
{
	if (index >= openCamers || !hasNewFrame[index].load(std::memory_order_acquire))
	{
		return false; // нет нового кадра Ц не блокируем мьютекс
	}
	std::lock_guard<std::mutex> lock(frameMutexes[index]);
	if (!lastFrames[index].empty())
	{
		lastFrames[index].copyTo(out);
		if (durationSinceStart) *durationSinceStart = timesFrame[index] - startTime;
	}
	hasNewFrame[index].store(false, std::memory_order_release); // помечаем как прочитанное
	return true;
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
			timesFrame[index] = std::chrono::steady_clock::now();
			hasNewFrame[index].store(true, std::memory_order_release);
		}
	}
	clearFrame.release();
}
void CameraManager::StopCameras_All()
{
	for (std::thread& th : threadsCameras)
	{
		if (th.joinable())
		{
			th.join();
		}
	}
	threadsCameras.clear();
	for (std::atomic_bool& flag : hasNewFrame)
	{
		flag = false;
	}
}

void CameraManager::SetParametersCameraStandard(cv::VideoCapture& camera) const
{
	camera.set(cv::CAP_PROP_FRAME_WIDTH, WidthFrames);
	camera.set(cv::CAP_PROP_FRAME_HEIGHT, HeightFrames);
	camera.set(cv::CAP_PROP_BITRATE, bitrate);
	camera.set(cv::CAP_PROP_FPS, FPSStandard);
	camera.set(cv::CAP_PROP_BRIGHTNESS, 0);
}
void CameraManager::SetParametersCameraScreen(cv::VideoCapture& camera) const
{
	camera.set(cv::CAP_PROP_FRAME_WIDTH, WidthFrames);
	camera.set(cv::CAP_PROP_FRAME_HEIGHT, HeightFrames);
	camera.set(cv::CAP_PROP_BITRATE, bitrate);
	camera.set(cv::CAP_PROP_FPS, FPSScreenCamera);// ќтключаем автоматическую экспозицию (переход в ручной режим)
	// ƒл€ Windows (DirectShow) обычно: 0 - авто, 1 - ручной[reference:0]
	camera.set(cv::CAP_PROP_AUTO_EXPOSURE, 1);
	// ”станавливаем значение экспозиции (значение зависит от камеры, часто в секундах или относительных единицах)[reference:1]
	camera.set(cv::CAP_PROP_EXPOSURE, 0);
	camera.set(cv::CAP_PROP_CONTRAST, 2);
	camera.set(cv::CAP_PROP_BRIGHTNESS, 0);
	//camera.set(cv::CAP_PROP_SETTINGS, 1); // ќткроет окно настроек драйвера[reference:7]
}

std::string CameraManager::timeToString(const Duration& time)
{
	using namespace std::chrono;
	auto totalMs = duration_cast<milliseconds>(time).count();
	auto totalSec = totalMs / 1000;
	auto hours = totalSec / 3600;
	auto minutes = (totalSec % 3600) / 60;
	auto seconds = totalSec % 60;
	auto millis = totalMs % 1000;

	std::stringstream ss;
	ss << std::setfill('0') << std::setw(2) << hours << ":"
		<< std::setw(2) << minutes << ":"
		<< std::setw(2) << seconds << "."
		<< std::setw(3) << millis;
	return ss.str();
}