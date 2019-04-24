#include "downloadsession.h"

DownloadSession::DownloadSession(QObject *parent)
    : QObject(parent)
    , fileSize(-1)
    , manager(nullptr)
    , bytesAllThreadRecevied(0)
    , finishCounter(0)
    , timeInterval(0)
{
    //启用计时器
    time.start();
    //连接信号
    connect(this, &DownloadSession::signalCheckDownloadProgress, this, &DownloadSession::onCheckDownloadProgress);
    connect(this, &DownloadSession::signalAllSubDownloadFinished, this, &DownloadSession::combineFile);
}

//析构函数
DownloadSession::~DownloadSession()
{
    delete [] manager;
    delete [] bytesSubThreadRecevied;
    delete [] mutexOnBytesSubThreadRecevied;
}
//初始化函数
void DownloadSession::init(int threadNum, int sessionId
                           , QString url, QString fileName, QString path)
{
    this->threadNum = threadNum;
    this->sessionId = sessionId;
    this->url = url;
    this->fileName = fileName;
    this->path = path;
    manager = new DownloadManager[threadNum];
    bytesSubThreadRecevied = new qint64[threadNum];
    mutexOnBytesSubThreadRecevied = new QMutex[threadNum];
}
//使用HEAD请求获取下载文件的相关信息
void DownloadSession::getFileInformation()
{
    bool isGetResult = false;
    int tryTimes = 5;
    do
    {
        QNetworkAccessManager manager;
        QEventLoop loop;//等待请求头文件信息结束的事件循环
        QTimer timer;//计时器，超时结束事件循环
        //发出请求
        QNetworkReply *reply = manager.head(QNetworkRequest(url));
        if(!reply)
            continue;
        //连接信号
        QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
        timer.start(2000);
        loop.exec();

        if(reply->error() != QNetworkReply::NoError)
        {
            //请求发生错误
            qDebug() << reply->errorString();
            continue;
        }
        else if (!timer.isActive())
        {
            //请求超时
            qDebug() << "Request Timeout." << endl;
            continue;
        }
        timer.stop();
        isGetResult = true;
        //获取文件大小
        QVariant var = reply->header(QNetworkRequest::ContentLengthHeader);
        fileSize = var.toLongLong();
        if(fileName == "")
        {
            //获取文件名：当存在filename字段时采用filename字段，当其不存在时根据URL返回文件名
            QString strDisposition = reply->rawHeader("Content-Disposition");
            int index = strDisposition.indexOf("filename=");
            fileName = strDisposition.mid(index + 9);
            if (fileName.isEmpty())
            {
                QUrl qurl(url);
                QFileInfo fileInfo(qurl.path());
                fileName += fileInfo.fileName();
            }
            fileName = QUrl::fromPercentEncoding(fileName.toUtf8());
        }
        qDebug() << fileName << ":" << transformUnit(fileSize) << endl;
        reply->deleteLater();//垃圾回收
        return;
    }while(tryTimes--);
}
//获取文件名
QString DownloadSession::getFileName()
{
    return fileName;
}
//获取文件大小
qint64 DownloadSession::getFileSize()
{
    return fileSize;
}
//字节单位转换
QString DownloadSession::transformUnit(qint64 bytes)
{
    double dBytes = bytes;
    QString strUnit;//单位
   if (dBytes <= 0)
   {
       dBytes = 0;
       strUnit = " B";
   }
   else if (dBytes < KB)
   {
       strUnit = " B";
   }
   else if (dBytes < MB)
   {
       dBytes /= KB;
       strUnit = " KB";
   }
   else if (dBytes < GB)
   {
       dBytes /= MB;
       strUnit = " MB";
   }
   else if (dBytes > GB)
   {
       dBytes /= GB;
       strUnit = " GB";
   }
   //保留小数点后两位
   return QString("%1%2").arg(QString::number(dBytes,'f',2)).arg(strUnit);
}
//时间单位转换
QString DownloadSession::transformTime(qint64 seconds)
{
    QString strValue;
    QString strSpacing(" ");
    if (seconds <= 0)
    {
        strValue = QString("%1s").arg(0);
    }
    else if (seconds < 60)
    {
        strValue = QString("%1s").arg(seconds);
    }
    else if (seconds < 60 * 60)
    {
        qint64 nMinute = seconds / 60;
        qint64 nSecond = seconds - nMinute * 60;

        strValue = QString("%1m").arg(nMinute);

        if (nSecond > 0)
            strValue += strSpacing + QString("%1s").arg(nSecond);
    }
    else if (seconds < 60 * 60 * 24)
    {
        qint64 nHour = seconds / (60 * 60);
        qint64 nMinute = (seconds - nHour * 60 * 60) / 60;
        qint64 nSecond = seconds - nHour * 60 * 60 - nMinute * 60;

        strValue = QString("%1h").arg(nHour);

        if (nMinute > 0)
            strValue += strSpacing + QString("%1m").arg(nMinute);
        if (nSecond > 0)
            strValue += strSpacing + QString("%1s").arg(nSecond);
    }
    else
    {
            qint64 nDay = seconds / (60 * 60 * 24);
            qint64 nHour = (seconds - nDay * 60 * 60 * 24) / (60 * 60);
            qint64 nMinute = (seconds - nDay * 60 * 60 * 24 - nHour * 60 * 60) / 60;
            qint64 nSecond = seconds - nDay * 60 * 60 * 24 - nHour * 60 * 60 - nMinute * 60;

            strValue = QString("%1d").arg(nDay);

            if (nHour > 0)
                strValue += strSpacing + QString("%1h").arg(nHour);
            if (nMinute > 0)
                strValue += strSpacing + QString("%1m").arg(nMinute);
            if (nSecond > 0)
                strValue += strSpacing + QString("%1s").arg(nSecond);
        }

        return strValue;
    }

//使用Range请求查询下载是否支持断点续传来确定能否使用多线程下载
bool DownloadSession::supportBreak()
{
    QNetworkAccessManager manager;
    QEventLoop loop;//等待请求头文件信息结束的事件循环
    QTimer timer;//计时器，超时结束事件循环
    //定义Range请求
    QNetworkRequest *request = new QNetworkRequest();
    request->setUrl(url);
    request->setRawHeader("Range","bytes=0-");
    //发出请求获取头部信息
    QNetworkReply *reply = manager.get(*request);
    if(!reply)
        return false;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    QObject::connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);

    timer.start(2000);
    loop.exec(QEventLoop::ExcludeUserInputEvents);

    if(reply->error() != QNetworkReply::NoError)
    {
        //请求发生错误
        qDebug() << reply->errorString();
        return false;
    }
    else if (!timer.isActive())
    {
        //请求超时
        qDebug() << "Request Timeout." << endl;
        return false;
    }

    timer.stop();
    QString range = reply->rawHeader("Content-Range");
    qDebug() << range << endl;
    if (!range.isEmpty())
        return true;
    return false;
}
//开始下载任务
void DownloadSession::startDownload()
{
    DownloadThread thread;//新建阻塞线程
    getFileInformation();//获取文件信息
    //根据线程数分配任务给各个DownloadManager
    for(int i = 0; i < threadNum; i++)
    {
        qint64 start = fileSize * i / threadNum;
        qint64 end = fileSize * (i+1) / threadNum;
        if (i+1 != threadNum)
            manager[i].init(i, url, path+fileName, start, end-1);
        else
            manager[i].init(i, url, path+fileName, start, end);
        connect(&manager[i], &DownloadManager::signalFinished, this, &DownloadSession::onSubFinished);
        connect(&manager[i], &DownloadManager::signalSubDownloadProgress, this, &DownloadSession::onSubDownloadProgress);
        connect(&manager[i], &DownloadManager::signalError, this, &DownloadSession::onError);
        connect(this, &DownloadSession::signalStopDownload, &manager[i], &DownloadManager::onStopDownload);
        connect(this, &DownloadSession::signalCancelDownload, &manager[i], &DownloadManager::onCancelDownload);
        manager[i].downloadFile();
    }
    connect(this, &DownloadSession::signalAllFinished, &thread, &DownloadThread::onQuitLoop);
    connect(this, &DownloadSession::signalStopDownload, &thread, &DownloadThread::onQuitLoop);
    connect(this, &DownloadSession::signalCancelDownload, &thread, &DownloadThread::onQuitLoop);
    connect(this, &DownloadSession::signalStartThreads, &thread, &DownloadThread::run);
    //初始化计时器与上一次记录的时间
    time.restart();
    timeInterval = 0;

}
//合并文件
void DownloadSession::combineFile()
{
    fileName = path + fileName;
    QString originFileName = fileName;
    QFileInfo fileinfo(fileName);
    if(fileinfo.exists())
    {
        int index = fileName.lastIndexOf(".");
        int tmpCounter = 1;
        QString tmpFileName = fileName;
        while(QFileInfo(tmpFileName).exists())
        {
            tmpFileName = fileName.left(index)
                    + " (" + QString::number(tmpCounter++) +")"
                    + "." +fileinfo.suffix();
        }
        fileName = tmpFileName;
    }
    QFile file(fileName);
    if(file.open(QFile::WriteOnly | QIODevice::Append))
    {
        QDataStream out(&file);
        for (int i = 0; i < threadNum; i++)
        {
            QString subFileName(originFileName+FILE_SUFFIX+QString::number(i));
            QFile subFile(subFileName);
            if( subFile.open(QFile::ReadOnly))
            {
                file.write(subFile.readAll());
            }
            subFile.close();
            QFile::remove(subFileName);
        }
    }
    file.close();
    emit signalAllFinished();
    qDebug() << "File Combined. "<< endl;
}
//当部分任务完成时，计数器finishCounter+1，判断是否所有线程都已完成
//如果是，发送signalAllSubDownloadFinished信号
void DownloadSession::onSubFinished()
{
    finishCounter++;
    if(finishCounter == threadNum)
    {
        emit signalAllSubDownloadFinished();
        qDebug() << "ALL FINISHED." << endl;
    }
}
//获取子线程下载信息，并发出确认总下载状态的信号
void DownloadSession::onSubDownloadProgress(int id, qint64 bytesReceived, qint64 bytesTotal)
{
    mutexOnBytesSubThreadRecevied[id].lock();//对id相对应的变量上锁
    this->bytesSubThreadRecevied[id] = bytesReceived;//改写变量
    mutexOnBytesSubThreadRecevied[id].unlock();//解锁
    emit signalCheckDownloadProgress();
}
//错误处理信号槽
void DownloadSession::onError(QNetworkReply::NetworkError errorCode)
{
    if(errorCode != QNetworkReply::NoError)
        emit signalCancelDownload();
}
//暂停任务信号槽
void DownloadSession::onPauseTask()
{
    emit signalStopDownload();
}
//取消任务信号槽
void DownloadSession::onCancelTask()
{
    emit signalCancelDownload();
}
//确认总下载状态
void DownloadSession::onCheckDownloadProgress()
{
    int timeNow = time.elapsed();//获取当前时间
//    qDebug() << timeNow << ":" << timeInterval;
    if (timeNow - timeInterval > TIME_INTERVAL)//每300ms刷新一次总下载状态（根据TIME_INTERVAL调整）
    {
        qint64 bytesAllThreadReceivedBefore = bytesAllThreadRecevied;//记录刷新前的字节数
        bytesAllThreadRecevied = 0;//清零
        for (int i = 0; i < threadNum; i++) {//使用for循环依次获取各个线程当前下载字节数
            mutexOnBytesSubThreadRecevied[i].lock();//对id相对应的变量上锁
            bytesAllThreadRecevied+=bytesSubThreadRecevied[i];//读取变量并累加
            mutexOnBytesSubThreadRecevied[i].unlock();//解锁
        }
        //计算速度
        qint64 speed = (bytesAllThreadRecevied - bytesAllThreadReceivedBefore) * 1000 / (timeNow-timeInterval);
        //根据当前速度计算剩余时间
        qint64 remainingTime = (fileSize-bytesAllThreadRecevied) / speed;
        //将信息发送至上层GUI
        emit signalDownloadProgress(transformUnit(bytesAllThreadRecevied), transformUnit(speed)+"/s", transformTime(remainingTime));
        //记录本次记录时间
        timeInterval = timeNow;
    }

}
