#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include <QDebug>
#include <QtNetwork>
#include "newtaskdialog.h"
#include "downloadsession.h"

//MainWindow类，主要负责GUI与信息收集展示
//拥有DownloadSession类负责管理网络请求
//以及DownloadThread类负责提供网络请求所需的阻塞线程

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);//构造函数
    ~MainWindow();//析构函数
private:
    //按键
    QAction *newTaskAction;//新建任务按钮
    QAction *continueTaskAction;//继续任务按钮
    QAction *pauseTaskAction;//暂停任务按钮
    QAction *cancelTaskAction;//取消任务按钮
    QAction *openFolderAction;//打开文件夹按钮
    QAction *exitAction;//退出按钮

    //工具栏
    QToolBar *toolBar;
    //空白填充
    QWidget *spacer;
    //主视图
    QGroupBox *view;
    //主要信息
    QLabel *nameLabel;//文件名
    QLabel *sizeLabel;//文件大小
    QLabel *speedLabel;//下载速度
    QLabel *timeLabel;//剩余时间
    //具体信息的承载Label
    QLabel *nameInfoLabel;
    QLabel *sizeInfoLabel;
    QLabel *speedInfoLabel;
    QLabel *timeInfoLabel;
    //布局
    QVBoxLayout *labelLayout;
    QVBoxLayout *infoLabelLayout;
    QHBoxLayout *centralLayout;
    //新建任务对话框
    NewTaskDialog *newTaskdia;
    //文件路径
    QUrl path;
    //DownloadSession类
    DownloadSession *session;
    QTime time;//计时器
    //DownloadThread类
    DownloadThread *thread;
private slots:
    //信号槽
    void newTask();//新建任务
    void continueTask();//继续任务
    void pauseTask();//暂停任务
    void cancelTask();//取消任务
    void openFolder();//打开文件
    void exit();//退出
public slots:
    void onDownloadInfoToMainProcess(QString url
                                     , QString filename
                                     , QString path
                                     , int threadNum);//获取下载信息
    void onAllFinished();//下载完成信号槽
    void onDownloadProgress(QString bytesReceived
                            , QString speed
                            , QString remainingTime);//获取下载进度
signals:
    void exitApp();//退出信号
    void signalContinueTask();//继续任务信号
    void signalPauseTask();//暂停任务信号
    void signalCancelTask();//取消任务信号
    void signalGetDownloadInfo();//获取文件信息信号
};

#endif // MAINWINDOW_H
