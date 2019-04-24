#ifndef DOWNLOADTHREAD_H
#define DOWNLOADTHREAD_H

#include <QtWidgets>
#include <QThread>
#include "downloadmanager.h"

//DownloadThread是为各个线程进行异步网络请求时提供的阻塞线程
//自身仅有一个与DownloadManager相关联的阻塞类QEventLoop
//由于各个下载线程均依靠同一个阻塞线程，各个下载线程之间为同步

class DownloadThread :public QThread
{
    Q_OBJECT
private:
    QEventLoop *loop;

public:
    DownloadThread(QObject *parent = nullptr);
    void run();//线程执行函数
public slots:
    void onQuitLoop();//停止阻塞信号槽
signals:
    void signalQuitLoop();
};

#endif // DOWNLOADTHREAD_H
