#include "downloadmanager.h"

DownloadManager::DownloadManager(QObject *parent)
    : QObject(parent)
    , id(0)
    , url("")
    , filename("")
    , rangeStart(0)
    , rangeEnd(0)
    , isStop(true)
    , supportBreakPoint(false)
    , multiThread(false)
    , finished(false)
    , bytesReceived(0)
    , bytesTotal(0)
    , bytesCurrentReceived(0)
{
    manager = new QNetworkAccessManager();//初始化负责网络访问的manager变量
}

DownloadManager::~DownloadManager()
{}

//对线程id、下载网址与文件名进行初始化
void DownloadManager::init(int id, QString url, QString filename)

{
    reset();
    qDebug() << "________" << id << "_________" << endl;
    this->id = id;
    this->url = QUrl(url);
    this->filename = filename;
    qDebug() << "Download Manager Initialized." << endl;
}
//除了前三个以外，对多线程下载所分配的字节起始到终止位置同样进行类初始化
void DownloadManager::init(int id, QString url, QString filename
                           , qint64 rangeStart, qint64 rangeEnd)

{
    reset();
    qDebug() << "________" << id << "_________" << endl;
    this->id = id;
    this->url = QUrl(url);
    this->filename = filename;
    this->rangeStart = rangeStart;
    this->rangeEnd = rangeEnd;
    this->multiThread = true;//由于分配了Range范围，确定本次为多线程下载中的其中一个线程
    qDebug() << "Download Manager Initialized." << endl;
}
//根据上级类确定本次传输是否支持断点续传
void DownloadManager::setIfSupportBreakPoint(bool supportBreakPoint)
{
    this->supportBreakPoint = supportBreakPoint;
}

void DownloadManager::downloadFile()
{
    if(isStop)
    {
        qDebug() << "Start Downloading File:" << endl;
        isStop = false;
        filename += FILE_SUFFIX + QString::number(id);//将文件名临时改为 原名+临时后缀+线程id 的形式
        qDebug() << "tmp File Name:" << filename << endl;
        if(bytesCurrentReceived == 0)
            removeFile();
        if(finished)
            return;
        QNetworkRequest request(url);//定义网络请求
        if(multiThread)//当确认为多线程下载时分配需要下载的字节段
        {
            QString range = QString("bytes=%1-%2").arg(rangeStart+bytesCurrentReceived).arg(rangeEnd);
            qDebug() << range;
            request.setRawHeader("Range", range.toUtf8());
        }
        reply = manager->get(request);//通过manager进行get请求
        //链接相关信号与信号槽
        connect(reply, &QNetworkReply::downloadProgress, this, &DownloadManager::onDownloadProgress);
        connect(reply, &QNetworkReply::readyRead,        this, &DownloadManager::onReadyRead);
        connect(reply, &QNetworkReply::finished,         this, &DownloadManager::onFinished);
        connect(reply, SIGNAL(error(QNetworkReply::NetworkError)),  this, SLOT(onError(QNetworkReply::NetworkError)));
    }
}


//暂停下载
void DownloadManager::stop()
{
    isStop = true;
    if(reply != nullptr)
    {
        //断开链接
        disconnect(reply, &QNetworkReply::downloadProgress, this, &DownloadManager::onDownloadProgress);
        disconnect(reply, &QNetworkReply::readyRead,        this, &DownloadManager::onReadyRead);
        disconnect(reply, &QNetworkReply::finished,         this, &DownloadManager::onFinished);
        disconnect(reply, SIGNAL(error(QNetworkReply::NetworkError)),  this, SLOT(onError(QNetworkReply::NetworkError)));
        reply->abort();
        reply->deleteLater();
        reply = nullptr;
    }
    qDebug() << "stop Func";
}
//删除文件
void DownloadManager::removeFile()
{
    QFileInfo fileInfo(filename);
    if(fileInfo.exists())
        QFile::remove(filename);
    qDebug() << "removeFile Func";
}
//重置参数
void DownloadManager::reset()
{
    bytesTotal = 0;
    bytesReceived = 0;
    bytesCurrentReceived = 0;
    finished = false;
    qDebug() << "reset Func";
}
//由于网络连接不确定性，允许一定范围内的重试链接
void DownloadManager::retryDownload()
{
    downloadFile();
}
//获取下载信息的信号槽
void DownloadManager::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    if(!isStop)
    {
        this->bytesReceived = bytesReceived;
        this->bytesTotal = bytesTotal;
        //通过发射信号的形式将已下载字节数与总字节数传递到上层
        emit signalSubDownloadProgress(id, bytesReceived + bytesCurrentReceived
                                    , bytesTotal + bytesCurrentReceived);
    }
}
//文件写入信号槽
void DownloadManager::onReadyRead()
{
    QFile file(filename);
    if(file.open(QIODevice::WriteOnly | QIODevice::Append))
    {
        file.write(reply->readAll());
    }
    file.close();
}
//下载完成反应信号槽
void DownloadManager::onFinished()
{
    qDebug() << id <<":  Download Finished." << endl;
    isStop = true;
    QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
    if(reply->error() != QNetworkReply::NoError)
    {
        QString strError = reply->errorString();
        qDebug() << "!!!____" << strError << "____!!!" << endl;
    }
    finished = true;
    emit signalFinished(statusCode.toInt());
    reply->deleteLater();
    reply = nullptr;
}
//错误处理信号槽
void DownloadManager::onError(QNetworkReply::NetworkError errorCode)
{
    if(retry--)
        retryDownload();
    else
        emit signalError(errorCode);
    qDebug() << "Error String:" << reply->errorString();
}
//停止下载信号槽
void DownloadManager::onStopDownload()
{
    if (!isStop)
    {
        bytesCurrentReceived +=bytesReceived;
        stop();
    }
}
//取消下载信号槽
void DownloadManager::onCancelDownload()
{
    stop();
    reset();
    removeFile();
}
