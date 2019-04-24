#ifndef DOWNLOADSESSION_H
#define DOWNLOADSESSION_H
#include <iostream>
#include <fstream>
#include <QObject>
#include <QtNetwork>
#include <QDebug>
#include "downloadmanager.h"
#include "downloadthread.h"

//DownloadSession为http下载的总管理类
//核心为DownloadManager类数组

using namespace std;
//常值定义
const double KB = 1024;
const double MB = 1024*1024;
const double GB = 1024*1024*1024;
const int TIME_INTERVAL = 300;

class DownloadSession : public QObject
{
    Q_OBJECT
private:
    int threadNum;//下载任务线程数
    int sessionId;//会话ID
    QString url;//下载地址
    QString fileName;//文件名
    QString path;//保存地址
    qint64 fileSize; //qint64 == long long 文件大小
    DownloadManager *manager;//下载线程指针
    qint64 *bytesSubThreadRecevied;//当前各个线程下载的字节数
    QMutex *mutexOnBytesSubThreadRecevied;//bytesAllThreadRecevied的读写访问互斥锁
    qint64 bytesAllThreadRecevied;//当前所有线程下载的字节数
    int finishCounter;//完成线程数量，初始化为0
    QTime time;//计时器
    int timeInterval;//上一次记录的时间
public:
    DownloadSession(QObject *parent = nullptr);//构造函数
    void init(int threadNum, int sessionId, QString url
              , QString fileName, QString path);//初始化函数
    ~DownloadSession();//析构函数
    void getFileInformation();//获取文件信息
    QString getFileName();//获取文件名
    qint64 getFileSize();//获取文件大小
    QString transformUnit(qint64 bytes);//单位转换
    QString transformTime(qint64 seconds);
    bool supportBreak();//检查是否支持断点续传
    void startDownload();//开始下载任务
    void combineFile();//合并文件
public slots:
    void onSubFinished();//部分完成信号槽
    void onSubDownloadProgress(int id, qint64 bytesReceived, qint64 bytesTotal);//获取下载文件信息
    void onError(QNetworkReply::NetworkError errorCode);//错误处理
    void onPauseTask();//暂停任务
    void onCancelTask();//取消任务
    void onCheckDownloadProgress();//确认下载状态
signals:
    void signalStartThreads();//开始任务信号
    void signalStopDownload();//停止任务信号
    void signalCancelDownload();//取消任务信号
    void signalAllSubDownloadFinished();//所有部分全部下载完成信号
    void signalAllFinished();//所有工作完成，本次下载结束信号
    void signalCheckDownloadProgress();//确认下载状态信号
    void signalDownloadProgress(QString bytesReceived
                                , QString speed
                                , QString remainingTime);//向GUI发送转换后的下载状态的信号
};
#endif // DOWNLOADSESSION_H
