#include "MainWindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStandardPaths>
#include <QDir>
#include <QDateTime>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUi();
    applyStyles();
    
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &MainWindow::updateFrame);
    
    // Try opening default camera
    setupCamera(0);
}

MainWindow::~MainWindow() {
    if (m_cap.isOpened()) {
        m_cap.release();
    }
    if (m_writer.isOpened()) {
        m_writer.release();
    }
}

void MainWindow::setupUi() {
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // Top Bar
    QWidget *topBar = new QWidget();
    topBar->setObjectName("topBar");
    QHBoxLayout *topLayout = new QHBoxLayout(topBar);
    topLayout->setContentsMargins(20, 10, 20, 10);
    
    m_cameraSelector = new QComboBox();
    m_cameraSelector->setMinimumWidth(200);
    // Populate with 5 possible slots
    for(int i=0; i<5; ++i) {
        m_cameraSelector->addItem(QString("Camera %1").arg(i));
    }
    topLayout->addWidget(m_cameraSelector);
    topLayout->addStretch();
    
    // Viewfinder
    m_viewfinder = new QLabel();
    m_viewfinder->setAlignment(Qt::AlignCenter);
    m_viewfinder->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_viewfinder->setMinimumSize(640, 480);
    // Dark background for viewfinder area
    m_viewfinder->setStyleSheet("background-color: #000;");

    // Bottom Bar
    QWidget *bottomBar = new QWidget();
    bottomBar->setObjectName("bottomBar");
    QHBoxLayout *bottomLayout = new QHBoxLayout(bottomBar);
    bottomLayout->setContentsMargins(20, 20, 20, 20);
    bottomLayout->setSpacing(20);

    m_captureButton = new QPushButton("Capture");
    m_captureButton->setObjectName("captureButton");
    m_captureButton->setFixedSize(120, 50);

    m_recordButton = new QPushButton("Record");
    m_recordButton->setObjectName("recordButton");
    m_recordButton->setCheckable(true);
    m_recordButton->setFixedSize(120, 50);

    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setStyleSheet("color: #888;");
    m_statusLabel->setAlignment(Qt::AlignCenter);

    bottomLayout->addStretch();
    bottomLayout->addWidget(m_recordButton);
    bottomLayout->addWidget(m_captureButton);
    bottomLayout->addStretch();

    mainLayout->addWidget(topBar);
    mainLayout->addWidget(m_viewfinder);
    mainLayout->addWidget(bottomBar);
    mainLayout->addWidget(m_statusLabel);

    connect(m_cameraSelector, QOverload<int>::of(&QComboBox::activated), 
            this, &MainWindow::changeCamera);
    connect(m_captureButton, &QPushButton::clicked, this, &MainWindow::captureImage);
    connect(m_recordButton, &QPushButton::clicked, this, &MainWindow::toggleRecording);
}

void MainWindow::setupCamera(int index) {
    if (m_timer->isActive()) m_timer->stop();
    if (m_cap.isOpened()) m_cap.release();

    if (m_cap.open(index)) {
        // Set basic props
        m_cap.set(cv::CAP_PROP_FRAME_WIDTH, 1280);
        m_cap.set(cv::CAP_PROP_FRAME_HEIGHT, 720);
        m_timer->start(30); // ~30 FPS
        m_statusLabel->setText(QString("Camera %1 Connected").arg(index));
    } else {
        m_statusLabel->setText(QString("Failed to open Camera %1").arg(index));
        // Try to clear the viewfinder
        m_viewfinder->clear();
        m_viewfinder->setText("No Signal");
    }
}

void MainWindow::changeCamera(int index) {
    setupCamera(index);
}

void MainWindow::updateFrame() {
    if (m_cap.isOpened()) {
        m_cap >> m_currentFrame;
        if (!m_currentFrame.empty()) {
            
            // Handle Recording
            if (m_isRecording && m_writer.isOpened()) {
                m_writer.write(m_currentFrame);
            }

            // Update UI
            // Resize image to fit label while keeping aspect ratio is expensive every frame, 
            // so we scale the pixmap at the end or use ScaledContents (which distorts).
            // Better: Scale QImage to fit m_viewfinder size.
            
            QImage qimg = matToQImage(m_currentFrame);
            
            // Scale keeping aspect ratio
            QPixmap p = QPixmap::fromImage(qimg);
            
            // Use setPixmap. If we want it to resize with window, we need to handle resize events,
            // but for now, we just let QLabel handle it or scale it here.
            // To prevent distortion with setScaledContents(true), we do manually:
            int w = m_viewfinder->width();
            int h = m_viewfinder->height();
            m_viewfinder->setPixmap(p.scaled(w, h, Qt::KeepAspectRatio, Qt::SmoothTransformation));
        }
    }
}

void MainWindow::captureImage() {
    if (m_currentFrame.empty()) return;

    QString path = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if (path.isEmpty()) path = QDir::homePath();
    
    QString fileName = path + "/PyPixl_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".png";
    
    // Save current frame (convert BGR to RGB first for QImage save, or use opencv imwrite)
    // Using OpenCV imwrite is safer for formats
    cv::imwrite(fileName.toStdString(), m_currentFrame);
    
    m_statusLabel->setText("Saved: " + fileName);
}

void MainWindow::toggleRecording() {
    if (!m_isRecording) {
        // Start Recording
        QString path = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
        if (path.isEmpty()) path = QDir::homePath();
        
        QString fileName = path + "/PyPixl_" + QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss") + ".avi";
        
        int width = (int)m_cap.get(cv::CAP_PROP_FRAME_WIDTH);
        int height = (int)m_cap.get(cv::CAP_PROP_FRAME_HEIGHT);
        
        // MJPG codec
        m_writer.open(fileName.toStdString(), cv::VideoWriter::fourcc('M','J','P','G'), 30, cv::Size(width, height));
        
        if (m_writer.isOpened()) {
            m_isRecording = true;
            m_recordButton->setText("Stop");
            m_recordButton->setStyleSheet("background-color: #e74c3c; color: white; border: none; border-radius: 25px;");
            m_statusLabel->setText("Recording... " + fileName);
        } else {
            m_statusLabel->setText("Error: Could not start recording.");
            m_recordButton->setChecked(false);
        }
    } else {
        // Stop Recording
        m_isRecording = false;
        m_writer.release();
        m_recordButton->setText("Record");
        m_recordButton->setStyleSheet("");
        applyStyles(); // reset style
        m_statusLabel->setText("Recording Saved.");
    }
}

QImage MainWindow::matToQImage(const cv::Mat &mat) {
    if(mat.type() == CV_8UC3) {
        // Copy input Mat
        const uchar *qImageBuffer = (const uchar*)mat.data;
        // Create QImage with same dimensions as Input Mat
        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_RGB888);
        return img.rgbSwapped(); // OpenCV is BGR, Qt is RGB
    } else if(mat.type() == CV_8UC1) {
        const uchar *qImageBuffer = (const uchar*)mat.data;
        QImage img(qImageBuffer, mat.cols, mat.rows, mat.step, QImage::Format_Grayscale8);
        return img;
    }
    return QImage();
}

void MainWindow::applyStyles() {
    QString qss = R"(
        QMainWindow {
            background-color: #1e1e1e;
        }
        QWidget#topBar, QWidget#bottomBar {
            background-color: #252526;
            border-top: 1px solid #3e3e42;
            border-bottom: 1px solid #3e3e42;
        }
        QComboBox {
            background-color: #333337;
            color: #f1f1f1;
            padding: 5px;
            border: 1px solid #3e3e42;
            border-radius: 4px;
        }
        QPushButton {
            background-color: #333337;
            color: #f1f1f1;
            border: 1px solid #3e3e42;
            border-radius: 25px;
            font-weight: bold;
            font-size: 14px;
        }
        QPushButton:hover {
            background-color: #3e3e42;
        }
        QPushButton#captureButton {
            background-color: #007acc;
            border: none;
        }
        QPushButton#captureButton:hover {
            background-color: #0098ff;
        }
        QLabel {
            font-family: 'Segoe UI', sans-serif;
            font-size: 12px;
            padding: 5px;
        }
    )";
    this->setStyleSheet(qss);
}