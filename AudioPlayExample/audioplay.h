#ifndef AUDIOPLAY_H
#define AUDIOPLAY_H
#define MP3_TYPE "mp3"
#define WAV_TYPE "wav"
#include <QObject>
#include <QDir>
#include <QFileDialog>
#include <QSound>
#include<QDebug>
#include<QMediaPlayer>
class AudioPlay : public QObject
{
    Q_OBJECT
public:
    explicit AudioPlay(QObject *parent = nullptr);
private:

signals:

    void DirectoryChanged(QString directory);
public slots:
    void OpenFile();
    void PlayAudio(QString filePath);
private:
    QMediaPlayer *player;




};

#endif // AUDIOPLAY_H
