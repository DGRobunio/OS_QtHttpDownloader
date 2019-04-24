#ifndef DOWNLOADMANAGER_H
#define DOWNLOADMANAGER_H
#include <iostream>
#include <fstream>
#include <QObject>
#include <QtNetwork>
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QFileInfo>

//DownloadManager类是本系统中的核心类，负责网络请求到文件下载的所有核心操作
//QT所带的QNetworkAccessManager类自身为异步请求，需要配合循环阻塞一同使用
//故DownloadManager类中没有明确开辟新线程的代码


const QString FILE_SUFFIX = "_TMP_";//下载文件的临时后缀之一

class DownloadManager :public QObject
{
    Q_OBJECT
private:
    int id;//线程id
    QNetworkAccessManager *manager;//Qt网络访问管理
    QNetworkReply *reply = nullptr;//接收网络返回内容的指针
    QUrl url;//下载网址
    QString filename;//文件路径+文件名
    qint64 rangeStart;//分段下载/断点续传起始点
    qint64 rangeEnd;//分段下载/断点续传结束点
    bool isStop;//是否暂停
    bool supportBreakPoint;//是否支持断点续传
    bool multiThread;//是否为多线程下载中的子线程
    bool finished;//任务是否完成
    qint64 bytesReceived;//已接收的字节数
    qint64 bytesTotal;//字节数总计
    qint64 bytesCurrentReceived;//当前已接收的字节数
    int retry = 5;//预设重试次数
public:
    DownloadManager(QObject *parent = nullptr);//构造函数
    void init(int id, QString url, QString filename);//初始化函数
    void init(int id, QString url, QString filename
              , qint64 rangeStart, qint64 rangeEnd);//初始化函数重载
    void setIfSupportBreakPoint(bool supportBreakPoint);//从上级获取该文件是否支持断点续传
    ~DownloadManager();//析构函数
    void downloadFile();//文件下载
    void stop();//停止http链接
    void removeFile();//移除文件
    void reset();//重制参数
    void retryDownload();//重试链接
public slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);//获取文件下载信息
    void onReadyRead();//获取下载内容，保存到文件中
    void onFinished();//下载完成
    void onError(QNetworkReply::NetworkError);//错误处理
    void onStopDownload();//暂停下载
    void onCancelDownload();//取消下载
signals:
    void signalSubDownloadProgress(int id
                                   , qint64 bytesReceived
                                   , qint64 bytesTotal);//文件下载信息信号
    void signalFinished(int statusCode);//任务完成信号
    void signalError(QNetworkReply::NetworkError errorCode);//错误信号

};

#endif // DOWNLOADMANAGER_H
