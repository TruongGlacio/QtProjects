#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<audioplay.h>
#include<qcustomplot.h>
#include<QDebug>
#include<audiodraw.h>
namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    void IntinialConnects(QWidget *parent);
    void DetroyConnects();
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    AudioPlay *audioPlay;
    QString fileDirect="";
    Audiodraw *audioDraw;

public slots:
    void SetFileDirect(QString fileDirect );
    void PlayAudio();
    void DrawWaveForm();
};

#endif // MAINWINDOW_H
