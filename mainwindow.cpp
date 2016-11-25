#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>

#include "SplitVideo.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->pushButton,&QPushButton::clicked,this,&MainWindow::showPlayDialog);
//    connect()
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::showPlayDialog(){
    qDebug() << "just a test";

    int beginInt = ui->lineEdit_2->text().toInt();
    int endInt = ui->lineEdit_3->text().toInt();

    qDebug() << beginInt << "beginbeginbeginbegin++";
    qDebug() << endInt << "endendendendendendend++";

    if(beginInt <= 0 || endInt <= 0){
        QMessageBox::warning(this,"警告","输入时间有误！");
        return;
    }
    if(beginInt > endInt){
        QMessageBox::warning(this,"警告","开始时间不能大于结束时间！");
        return;
    }

    QString savePath = QFileDialog::getSaveFileName(this,tr("保存视频路径"),"",
        tr("*.mp4")); //选择路径
    if(savePath.isEmpty()){
        return;
    }

    QString filePath = ui->lineEdit->text();
    if(filePath.isEmpty()){
        QMessageBox::warning(this,"警告","输入地址不能为空");
        return;
    }

    qDebug() << filePath << "747 inputfileinputfileinputfileinputfileinputfile";
    qDebug() << savePath << "748 outputfileoutputfileoutputfileoutputfileoutputfile";

    SplitVideo *sv = new SplitVideo(this);
    sv->setInOutName(filePath.toStdString(),savePath.toStdString());

    if (sv->executeSplit(beginInt,endInt)){
        qDebug() << "split success" ;
    }
    else{
        qDebug() << "failed" ;
    }
    delete sv;
}
