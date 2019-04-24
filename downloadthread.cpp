#include "downloadthread.h"

DownloadThread::DownloadThread(QObject *parent)
    :QThread (parent)
{
    loop = new QEventLoop;//定义阻塞类QEventLoop
    //当获取对应信号时退出阻塞
    connect(this, &DownloadThread::signalQuitLoop, loop, &QEventLoop::quit);
}

void DownloadThread::run()
{
    loop->exec(QEventLoop::ExcludeUserInputEvents);
}

void DownloadThread::onQuitLoop()
{
    emit signalQuitLoop();
}
