#include "audioplay.h"
#include<QFileInfo>
AudioPlay::AudioPlay(QObject *parent) : QObject(parent)
{
    player = new QMediaPlayer;
}

void AudioPlay::PlayAudio(QString filePath)
{
    QFileInfo fileInfo(filePath);
    QString extentionFile=fileInfo.completeSuffix();
    if(extentionFile==WAV_TYPE)
        QSound::play(filePath);
    else {
        player->setMedia(QUrl::fromLocalFile(filePath));
        player->setVolume(100);
        player->play();
    }

}

void AudioPlay::OpenFile()
{
    QWidget *parent=new QWidget();
    QString filePath;
    filePath =  QFileDialog::getOpenFileName( parent,"Open Media",QDir::currentPath(),
                        "All files (*.*) ;;Media file (*.mp3 *.wav);; Video files (*.mp4)");

       if (!filePath.isEmpty()) {

           emit DirectoryChanged(filePath.toUtf8());
       }
       else {
           emit DirectoryChanged("preDirectory");

       }
}

