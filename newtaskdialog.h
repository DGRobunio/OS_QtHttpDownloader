#ifndef NEWTASKDIALOG_H
#define NEWTASKDIALOG_H
#include <QtWidgets>

//NewTaskDialog类为QDialog类的子类
//用来提供新建下载的对话框

class NewTaskDialog : public QDialog
{
    Q_OBJECT
private:
    QLabel *urlLabel;
    QCheckBox *nameLabel;
    QLabel *addrLabel;
    QLabel *threadLabel;

    QLineEdit *urlLineEdit;
    QLineEdit *nameLineEdit;
    QLineEdit *addrLineEdit;

    QCheckBox *multiThreadCheckBox;
    QSpinBox *threadNumSpinBox;

    QWidget *spacer;

    QPushButton *confirmButton;
    QPushButton *cancelButton;

    QVBoxLayout *labelLayout;
    QVBoxLayout *lineEditLayout;
    QHBoxLayout *editLayout;
    QHBoxLayout *buttonLayout;
    QVBoxLayout *mainLayout;

public:
    NewTaskDialog(QWidget *parent = nullptr);

public slots:
    void accept();
    void urlChanged(QString);
    void nameLabelCheckBoxChanged(int);
    void multiThreadCheckBoxChanged(int);

signals:
    void signalDownloadInfoToMainProcess(QString url, QString filename, QString path, int threadNum);
};

#endif // NEWTASKDIALOG_H
