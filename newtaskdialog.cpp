#include "newtaskdialog.h"

NewTaskDialog::NewTaskDialog(QWidget *parent): QDialog (parent)
{
    urlLabel = new QLabel(tr("URL:"));
    nameLabel = new QCheckBox(tr("Customize Name:"));
    nameLabel->setCheckState(Qt::CheckState::Unchecked);
    addrLabel = new QLabel(tr("address:"));
    labelLayout = new QVBoxLayout();
    labelLayout->addWidget(urlLabel);
    labelLayout->addWidget(nameLabel);
    labelLayout->addWidget(addrLabel);
    connect(nameLabel, &QCheckBox::stateChanged, this, &NewTaskDialog::nameLabelCheckBoxChanged);

    urlLineEdit = new QLineEdit();
    urlLineEdit->setText("http://mirrors.163.com/debian/ls-lR.gz");
    nameLineEdit = new QLineEdit();
    nameLineEdit->setEnabled(false);
    addrLineEdit = new QLineEdit();
    addrLineEdit->setText(tr("/Users/DGR/Downloads/__TMP/"));
//    addrLineEdit->setText(tr("E:/"));
    lineEditLayout = new QVBoxLayout();
    lineEditLayout->addWidget(urlLineEdit);
    lineEditLayout->addWidget(nameLineEdit);
    lineEditLayout->addWidget(addrLineEdit);
    editLayout = new QHBoxLayout();
    editLayout->addLayout(labelLayout);
    editLayout->addLayout(lineEditLayout);
    connect(urlLineEdit, &QLineEdit::textChanged, this, &NewTaskDialog::urlChanged);

    spacer = new QWidget(this);
    spacer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    multiThreadCheckBox = new QCheckBox;
    multiThreadCheckBox->setText(tr("启用多线程"));
    multiThreadCheckBox->setCheckState(Qt::CheckState::Checked);
    threadNumSpinBox = new QSpinBox;
    threadNumSpinBox->setRange(1,16);
    threadNumSpinBox->setValue(8);
    connect(multiThreadCheckBox, &QCheckBox::stateChanged, this, &NewTaskDialog::multiThreadCheckBoxChanged);

    confirmButton = new QPushButton(tr("Confirm"));
//    confirmButton->setEnabled(false);
    connect(confirmButton, &QPushButton::pressed, this, &QDialog::accept);
    cancelButton = new QPushButton(tr("Cancel"));
    connect(cancelButton, &QPushButton::pressed, this, &QDialog::close);
    confirmButton->setDefault(true);

    buttonLayout = new QHBoxLayout();
    buttonLayout->addWidget(multiThreadCheckBox);
    buttonLayout->addWidget(threadNumSpinBox);
    buttonLayout->addWidget(spacer);
    buttonLayout->addWidget(cancelButton);
    buttonLayout->addWidget(confirmButton);

    mainLayout = new QVBoxLayout();
    mainLayout->addLayout(editLayout);
    mainLayout->addWidget(spacer);
    mainLayout->addLayout(buttonLayout);

    setLayout(mainLayout);

    setFixedSize(400, 200);
}

void NewTaskDialog::accept()
{
    QDialog::accept();
    if(nameLabel->isChecked())
        emit signalDownloadInfoToMainProcess(urlLineEdit->text()
                                             , nameLineEdit->text()
                                             , addrLineEdit->text()
                                             , threadNumSpinBox->value());
    else
        emit signalDownloadInfoToMainProcess(urlLineEdit->text()
                                             , ""
                                             , addrLineEdit->text()
                                             , threadNumSpinBox->value());
}

void NewTaskDialog::urlChanged(QString url) {
    if(url != "")
        confirmButton->setEnabled(true);
    else
        confirmButton->setEnabled(false);
}

void NewTaskDialog::nameLabelCheckBoxChanged(int status)
{
    if(status == Qt::CheckState::Checked)
        nameLineEdit->setEnabled(true);
    else
        nameLineEdit->setEnabled(false);
}

void NewTaskDialog::multiThreadCheckBoxChanged(int status)
{
    if(status == Qt::CheckState::Checked)
        threadNumSpinBox->setEnabled(true);
    else
    {
        threadNumSpinBox->setValue(1);
        threadNumSpinBox->setEnabled(false);
    }
}
