#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QComboBox>
#include <QPushButton>
#include <QTimer>
#include <QElapsedTimer>
#include <opencv2/opencv.hpp>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void updateFrame();
    void captureImage();
    void toggleRecording();
    void changeCamera(int index);

private:
    void setupUi();
    void setupCamera(int index);
    void applyStyles();
    QImage matToQImage(const cv::Mat &mat);

    // OpenCV objects
    cv::VideoCapture m_cap;
    cv::VideoWriter m_writer;
    cv::Mat m_currentFrame;
    bool m_isRecording = false;
    
    // Recording sync
    QElapsedTimer m_recordingTimer;
    qint64 m_lastRecordingTimeMs = 0;
    const double TARGET_FPS = 30.0;
    const double MS_PER_FRAME = 1000.0 / 30.0;

    // UI Elements
    QWidget *m_centralWidget = nullptr;
    QLabel *m_viewfinder = nullptr; // Used to display video
    QComboBox *m_cameraSelector = nullptr;
    QPushButton *m_captureButton = nullptr;
    QPushButton *m_recordButton = nullptr;
    QLabel *m_statusLabel = nullptr;
    
    QTimer *m_timer = nullptr;
};

#endif // MAINWINDOW_H