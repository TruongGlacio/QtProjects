#ifndef AUDIODRAW_H
#define AUDIODRAW_H

#include <QObject>
#include <QWidget>
#include "qcustomplot.h"
#include"wavfile.h"
class Audiodraw : public QWidget
{
    Q_OBJECT
public:
    explicit Audiodraw(QWidget *parent = nullptr);
    void DrawWaveForm(QString filePath,QCustomPlot* qcustomPlot);
    void ReadWavFile(QString filePath);

signals:

public slots:
    void realtimeDataSlot();
private:
    QTimer dataTimer;
    QCustomPlot* qcustomPlot;

};

#endif // AUDIODRAW_H
