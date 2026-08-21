#pragma once
#include <opencv2/opencv.hpp>
#include <chrono>

class CameraManager
{
public:
    using TimePoint = std::chrono::time_point<std::chrono::steady_clock>;
    using Duration = std::chrono::steady_clock::duration;
    /// <summary>Создание класса</summary>
    explicit CameraManager() {}
    /// <summary>Создание класса с заране указаным количеством камером</summary>
    /// <param name="index">Индексы камеры в системе</param>
    /// <param name="countIndex">Количество камер</param>
    explicit CameraManager(int index[], size_t countIndex)
    {
        CreatCameras(index, countIndex);
    }
    /// <summary>Диструктор</summary>
    ~CameraManager()
    {
        StopCameras_All();
        for (cv::VideoCapture& camera : cameras)
        {
            camera.release();
        }
        for (cv::Mat& frame : lastFrames)
        {
            frame.release();
        }
    }
    /// <summary>Запуск всех камер с выводом в окна OpenCV</summary>
    void StartCameras_ToCheckWork();
    /// <summary>Остановка камер/потоков на получения кадров</summary>
    void StopCameras_All();
    /// <summary>Создание камер</summary>
    /// <param name="index">Индексы камеры в системе</param>
    /// <param name="countIndex">Количество камер</param>
    void CreatCameras(int index[], size_t countIndex);
    bool getFrame(size_t index, cv::Mat& out, Duration* durationSinceStart = nullptr) const;
    bool getFrameWithReadyFlag(size_t index, cv::Mat& out, Duration* durationSinceStart = nullptr) const;
    bool getScreenFrame(cv::Mat& out, Duration* durationSinceStart = nullptr) const;
    bool getScreenFrameWithReadyFlag(cv::Mat& out, Duration* durationSinceStart = nullptr) const;

    CameraManager(const CameraManager&) = delete;
    CameraManager& operator=(const CameraManager&) = delete;
    CameraManager(CameraManager&&) = delete;
    CameraManager& operator=(CameraManager&&) = delete;

protected:
    /// <summary>Ширина кадров</summary>
    const double WidthFrames = 1280;
    /// <summary>Высота кадров</summary>
    const double HeightFrames = 720;
    /// <summary>Стандартный FPS камер</summary>
    const double FPSStandard = 60;
    /// <summary>FPS камеры на экран</summary>
    const double FPSScreenCamera = 120;
    /// <summary>Битрейт с которым записываются видео</summary>
    const double bitrate = 7500000;
private:
    /// <summary>Буфер для кадров/Вектор кадров получаемых с камер для передачи из потоков</summary>
    std::vector<cv::Mat> lastFrames;
    /// <summary>момент запуска камер</summary>
    TimePoint startTime;
    /// <summary>Вектор времени прихода кадра</summary>
    std::vector<TimePoint> timesFrame;
    /// <summary>Синхронизаторы потоков</summary>
    mutable std::deque<std::mutex> frameMutexes;
    /// <summary>Флаг, что кадр обновлён</summary>
    mutable std::deque<std::atomic<bool>> hasNewFrame;
    /// <summary>Вектор потоков для работы камер</summary>
    std::vector<std::thread> threadsCameras;
    /// <summary>Флаг для остановки всех потоков</summary>
    std::atomic<bool> workFlag = false;
    /// <summary>Массив камер, последняя считается камерой для лазера</summary>
    std::vector <cv::VideoCapture> cameras;
    /// <summary>Количество открытых и запущеных камер</summary>
    size_t openCamers = 0;
    /// <summary>Запуск потока камеры на получения кадров</summary>
    /// <param name="index">Индекс камеры в классе</param>
    void StartThreadReadFrame(int index);
    /// <summary>Считывания кадра с камеры и помещения в буфер</summary>
    /// <param name="index">Индекс камеры в классе</param>
    void ReadFrame(int index);
    /// <summary>Установка параметров для камеры, которая смотрит не на экран</summary>
    /// <param name="camera">Камера для настройки</param>
    void SetParametersCameraStandard(cv::VideoCapture& camera) const;
    /// <summary>Установка параметров для камеры, которая смотрит на экран</summary>
    /// <param name="camera">Камера для настройки</param>
    void SetParametersCameraScreen(cv::VideoCapture& camera) const;

    std::string timeToString(const Duration& time);
};