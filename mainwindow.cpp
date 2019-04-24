#include "mainwindow.h"
#include <QMenuBar>
#include <QMessageBox>
#include <QStatusBar>
#include <QToolBar>

//构造函数，主要用于构建GUI界面
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle(tr("Http下载器"));//设置主窗口名称

    //-------------定义按键&设置信号&信号槽---------------
    newTaskAction = new QAction(QIcon(":/images/new"), tr("&New Task"), this);
    newTaskAction->setShortcuts(QKeySequence::New);
    newTaskAction->setStatusTip(tr("New Task."));
    connect(newTaskAction, &QAction::triggered, this, &MainWindow::newTask);

    continueTaskAction = new QAction(QIcon(":/images/start"), tr("&Continue"), this);
    continueTaskAction->setStatusTip(tr("Continue Task"));
    continueTaskAction->setEnabled(false);
    connect(continueTaskAction, &QAction::triggered, this, &MainWindow::continueTask);

    pauseTaskAction = new QAction(QIcon(":/images/pause"), tr("&Pause"), this);
    pauseTaskAction->setStatusTip(tr("Pause"));
    pauseTaskAction->setEnabled(false);
    connect(pauseTaskAction, &QAction::triggered, this, &MainWindow::pauseTask);

    cancelTaskAction = new QAction(QIcon(":/images/cancel"), tr("&Cancel Task"), this);
    cancelTaskAction->setShortcuts(QKeySequence::Cancel);
    cancelTaskAction->setStatusTip(tr("Cancel Task."));
    cancelTaskAction->setEnabled(false);
    connect(cancelTaskAction, &QAction::triggered, this, &MainWindow::cancelTask);

    openFolderAction = new QAction(QIcon(":/images/open"), tr("&Open Download Folder"), this);
    openFolderAction->setShortcuts(QKeySequence::Open);
    openFolderAction->setStatusTip(tr("Open Download Folder."));
    openFolderAction->setEnabled(false);
    connect(openFolderAction, &QAction::triggered, this, &MainWindow::openFolder);


    exitAction = new QAction(QIcon(":/images/exit"), tr("&Exit"), this);
    exitAction->setShortcuts(QKeySequence::Quit);
    exitAction->setStatusTip(tr("Exit."));
    connect(exitAction, &QAction::triggered, this, &MainWindow::exit);

    //---------------设置工具栏按键-----------------
    toolBar = addToolBar(tr("&Task"));
    toolBar->addAction(newTaskAction);
    toolBar->addAction(continueTaskAction);
    toolBar->addAction(pauseTaskAction);
    toolBar->addAction(cancelTaskAction);
    toolBar->addAction(openFolderAction);
    spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);//设置为Expanding属性
    toolBar->addWidget(spacer);//工具栏添加空白条作填充，让最后一个按钮右顶格
    toolBar->addSeparator();
    toolBar->addAction(exitAction);
    toolBar->setMovable(false);//设置工具栏不可拖动

    view = new QGroupBox();

    nameLabel = new QLabel(tr("文件名 ："));
    nameInfoLabel = new QLabel();
    sizeLabel = new QLabel(tr("文件大小："));
    sizeInfoLabel = new QLabel();
    speedLabel = new QLabel(tr("下载速度："));
    speedInfoLabel = new QLabel();
    timeLabel = new QLabel(tr("剩余时间："));
    timeInfoLabel = new QLabel();

    labelLayout = new QVBoxLayout();
    labelLayout->addWidget(nameLabel);
    labelLayout->addWidget(sizeLabel);
    labelLayout->addWidget(speedLabel);
    labelLayout->addWidget(timeLabel);
    infoLabelLayout = new QVBoxLayout();
    infoLabelLayout->addWidget(nameInfoLabel);
    infoLabelLayout->addWidget(sizeInfoLabel);
    infoLabelLayout->addWidget(speedInfoLabel);
    infoLabelLayout->addWidget(timeInfoLabel);
    centralLayout = new QHBoxLayout();
    centralLayout->addLayout(labelLayout);
    centralLayout->addLayout(infoLabelLayout);
    view->setLayout(centralLayout);
    this->setCentralWidget(view);

    statusBar() ;//显示状态条
    resize(600, 300);//定义窗体尺寸

}

MainWindow::~MainWindow()
{

}
//新建任务信号槽
void MainWindow::newTask()
{
    //新建下载任务窗体
    newTaskdia = new NewTaskDialog(this);
    connect(newTaskdia, &NewTaskDialog::signalDownloadInfoToMainProcess, this, &MainWindow::onDownloadInfoToMainProcess);
    newTaskdia->open();
}
//继续任务信号槽
void MainWindow::continueTask()
{
    emit signalContinueTask();
    continueTaskAction->setEnabled(false);
    pauseTaskAction->setEnabled(true);
    cancelTaskAction->setEnabled(true);
}
//暂停任务信号槽
void MainWindow::pauseTask()
{
    emit signalPauseTask();
    continueTaskAction->setEnabled(true);
    pauseTaskAction->setEnabled(false);
    speedInfoLabel->setText(tr("0 B/s"));
    timeInfoLabel->setText((tr("0 s")));
}
//取消任务信号槽
void MainWindow::cancelTask()
{
    emit signalCancelTask();
    continueTaskAction->setEnabled(false);
    cancelTaskAction->setEnabled(false);
    pauseTaskAction->setEnabled(false);
    nameInfoLabel->setText("");
    sizeInfoLabel->setText("");
    speedInfoLabel->setText("");
    timeInfoLabel->setText("");
}
//打开文件夹信号槽
void MainWindow::openFolder()
{
    QDesktopServices::openUrl(QUrl::fromLocalFile(path.toString()));
}
//退出信号槽
void MainWindow::exit()
{
    emit exitApp();//发送exitApp信号
}
//获取下载信息信号槽
void MainWindow::onDownloadInfoToMainProcess(QString url, QString filename, QString path, int threadNum)
{
    nameInfoLabel->setText(tr("正在获取文件信息..."));
    sizeInfoLabel->setText(tr("0 B"));
    url = url.trimmed();//删除多余空白字符
    qDebug() << url << endl
             << filename << endl
             << path << endl;
    this->path = QUrl(path);
    openFolderAction->setEnabled(true);
    pauseTaskAction->setEnabled(true);
    cancelTaskAction->setEnabled(true);
    session = new DownloadSession();
    thread = new DownloadThread(this);
    session->init(threadNum,1,url,filename,path);

    connect(this, &MainWindow::signalContinueTask, session, &DownloadSession::startDownload);
    connect(this, &MainWindow::signalPauseTask, session, &DownloadSession::onPauseTask);
    connect(this, &MainWindow::signalCancelTask, session, &DownloadSession::onCancelTask);
    connect(session, &DownloadSession::signalAllFinished, this, &MainWindow::onAllFinished);
    connect(session, &DownloadSession::signalDownloadProgress, this, &MainWindow::onDownloadProgress);
    session->getFileInformation();
    nameInfoLabel->setText(session->getFileName());
    sizeInfoLabel->setText(QString("0 B/%1").arg(session->transformUnit(session->getFileSize())));
    session->startDownload();
    time.start();
}
//下载完成信号槽
void MainWindow::onAllFinished()
{
    nameLabel->setText(tr("文件位置："));
    sizeInfoLabel->setText(QString("%1").arg(session->transformUnit(session->getFileSize())));
    nameInfoLabel->setText(session->getFileName());
    speedInfoLabel->setText(tr("0 B/s"));
    timeInfoLabel->setText((tr("0 s")));
    continueTaskAction->setEnabled(false);
    pauseTaskAction->setEnabled(false);
    cancelTaskAction->setEnabled(false);
}
//下载进度信号槽
void MainWindow::onDownloadProgress(QString bytesReceived, QString speed, QString remainingTime)
{
    sizeInfoLabel->setText(QString("%1/%2")
                           .arg(bytesReceived)
                           .arg(session->transformUnit(session->getFileSize())));
    speedInfoLabel->setText(speed);
    timeInfoLabel->setText(remainingTime);
}
