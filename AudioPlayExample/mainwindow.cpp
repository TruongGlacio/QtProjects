#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    audioPlay=new AudioPlay();
    audioDraw=new Audiodraw();
    // intinial Connects only after intinilail audio play
    IntinialConnects(parent);
}

void MainWindow::IntinialConnects(QWidget *parent)
{
    connect(ui->pushButton_SelectFile,SIGNAL(released()),audioPlay, SLOT(OpenFile()));
    connect(audioPlay,SIGNAL(DirectoryChanged(QString)),this, SLOT(SetFileDirect(QString)));
    connect(ui->pushButtonPlay,SIGNAL(released()),this,SLOT(PlayAudio()));
    connect(ui->pushButtonPlay,SIGNAL(released()),this, SLOT(DrawWaveForm()));
}

void MainWindow::DetroyConnects()
{

}


MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SetFileDirect(QString fileDirect)
{
    this->fileDirect=fileDirect;
    ui->lineEditFilePath->setText(this->fileDirect);
}

void MainWindow::PlayAudio()
{
    if(fileDirect.isEmpty())
    {
           qDebug() << "File path not exits, can't play sound file";
           return;
    }

    audioPlay->PlayAudio(fileDirect);
}

void MainWindow::DrawWaveForm()
{
    audioDraw->DrawWaveForm(this->fileDirect,ui->customPlot);
}

