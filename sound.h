#ifndef SOUND_H
#define SOUND_H

#include <QObject>
#include <QAudioOutput>
#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <math.h>

class Sound : public QThread
{
    Q_OBJECT

public:
    explicit Sound(QObject *parent = nullptr);

    void beep(float freq_Hz, float duration_s);

    void requestInterruption();

    static const int SAMPLE_FREQ = 44100;
//    static const int AUDIO_BUF_MS = 30;

signals:
    void beepFinished();

protected:
    void run() override;

private:
    QAudioOutput *m_audio;
    QIODevice *m_device;
    QWaitCondition m_wait;

    float m_signal;
    float m_phase;

    float m_freq;
    float m_time;

//    void playbackNotify();
};

#endif // SOUND_H
