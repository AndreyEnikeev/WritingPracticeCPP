#pragma once
#include <opencv2/opencv.hpp>

class CameraManager
{
public:
    explicit CameraManager(int index[], size_t countIndex) {
        for (size_t i = 0; i < countIndex && i < countsCamers; i++)
        {
            cameras[i].open(index[i], cv::VideoCaptureAPIs::CAP_DSHOW);
            cameras[i].set(cv::CAP_PROP_FRAME_WIDTH, WidthFrames);
            cameras[i].set(cv::CAP_PROP_FRAME_HEIGHT, HeightFrames);
            cameras[i].set(cv::CAP_PROP_FPS, 60);
            cameras[i].set(cv::CAP_PROP_BITRATE, bitrate);
            openCamers++;
        }
    }
    /*Примеры для себя с vector
    explicit CameraManager(size_t count) : cameras(count) {
        for (size_t i = 0; i < count; ++i) {
            cameras[i].open(static_cast<int>(i));
        }
    }
    explicit CameraManager(const std::vector<int>& indices) : cameras(indices.size()) {
        for (size_t i = 0; i < indices.size(); ++i) {
            cameras[i].open(indices[i]);
        }
    }
    explicit CameraManager(const std::vector<int>& indices) {
        cameras.reserve(indices.size());
        for (int idx : indices) {
            cameras.emplace_back(idx);
        }
    }*/
    ~CameraManager()
    {
        StopCameras_All();
        for (cv::VideoCapture camera : cameras)
        {
            camera.release();
        }
    }
    void StartCameras_All();
    void StopCameras_All();



protected:
    /// <summary>Ширина кадров</summary>
    const int WidthFrames = 1280;
    /// <summary>Высота кадров</summary>
    const int HeightFrames = 720;
private:
    std::vector<std::thread> threadsCameras;
    std::atomic<bool> workFlag = false;
    /// <summary>Максимальное количество камер</summary>
    const size_t countsCamers = 4;
    /// <summary>Количество открытых и запущеных камер</summary>
    size_t openCamers = 0;
    /// <summary>Битрейт с которым записываются видео</summary>
    const int bitrate = 7500000;
    cv::Mat clearFrame;
    /// <summary>Массив камер</summary>
    /// <remarks>1 камера сбоку<para/>2 Перед<para/>3 Верх<para/>4 Экран </remarks>
    cv::VideoCapture cameras[4];
    void StartCamera(int index);
    void readFrame(cv::VideoCapture& camera);
};