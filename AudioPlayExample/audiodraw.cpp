#include "audiodraw.h"
Audiodraw::Audiodraw(QWidget *parent) : QWidget(parent)
{
    qcustomPlot=new QCustomPlot();

}

void Audiodraw::DrawWaveForm(QString filePath,QCustomPlot* qcustomPlot)
{

    qcustomPlot->addGraph(); // blue line
    qcustomPlot->graph(0)->setPen(QPen(QColor(40, 110, 255)));
   // qcustomPlot->addGraph(); // red line
  //  qcustomPlot->graph(1)->setPen(QPen(QColor(255, 110, 40)));

    QSharedPointer<QCPAxisTickerTime> timeTicker(new QCPAxisTickerTime);
    timeTicker->setTimeFormat("%h:%m:%s");
    qcustomPlot->xAxis->setTicker(timeTicker);
    qcustomPlot->axisRect()->setupFullAxesBox();
    qcustomPlot->yAxis->setRange(-10, 10);

    // make left and bottom axes transfer their ranges to right and top axes:
    connect(qcustomPlot->xAxis, SIGNAL(rangeChanged(QCPRange)), qcustomPlot->xAxis2, SLOT(setRange(QCPRange)));
    connect(qcustomPlot->yAxis, SIGNAL(rangeChanged(QCPRange)), qcustomPlot->yAxis2, SLOT(setRange(QCPRange)));

    // setup a timer that repeatedly calls MainWindow::realtimeDataSlot:
    this->qcustomPlot=qcustomPlot;
    connect(&dataTimer, SIGNAL(timeout()), this, SLOT(realtimeDataSlot()));
    dataTimer.start(0); // Interval 0 means to refresh as fast as possible

}

void Audiodraw::ReadWavFile(QString filePath)
{


}
void Audiodraw::realtimeDataSlot()
{
  static QTime time(QTime::currentTime());
  // calculate two new data points:
  double key = time.elapsed()/1000.0; // time elapsed since start of demo, in seconds
  static double lastPointKey = 0;
  if (key-lastPointKey > 0.002) // at most add point every 2 ms
  {
    // add data to lines:
    this->qcustomPlot->graph(0)->addData(key,qrand()/(double)RAND_MAX*20-10);//*qSin(key/3.843));
    this->qcustomPlot->graph(0)->addData(key,qrand()/(double)RAND_MAX*10-5);//*qSin(key/3.843));
    this->qcustomPlot->graph(0)->addData(key,qrand()/(double)RAND_MAX*5-2.5);//*qSin(key/3.843));
    this->qcustomPlot->graph(0)->addData(key,qrand()/(double)RAND_MAX-1);//*qSin(key/3.843));


 //  this->qcustomPlot->graph(1)->addData(key, qCos(key)+qrand()/(double)RAND_MAX*0.5*qSin(key/4.364));
    // rescale value (vertical) axis to fit the current data:
    //ui->customPlot->graph(0)->rescaleValueAxis();
    //ui->customPlot->graph(1)->rescaleValueAxis(true);
    lastPointKey = key;
  }
  // make key axis range scroll with the data (at a constant range size of 8):
  this->qcustomPlot->xAxis->setRange(key, 8, Qt::AlignRight);
  this->qcustomPlot->replot();

  // calculate frames per second:
  static double lastFpsKey;
  static int frameCount;
  ++frameCount;
//  if (key-lastFpsKey > 2) // average fps over 2 seconds
//  {
//    ui->statusBar->showMessage(
//          QString("%1 FPS, Total Data points: %2")
//          .arg(frameCount/(key-lastFpsKey), 0, 'f', 0)
//          .arg(ui->customPlot->graph(0)->data()->size()+ui->customPlot->graph(1)->data()->size())
//          , 0);
//    lastFpsKey = key;
//    frameCount = 0;
// }
}
