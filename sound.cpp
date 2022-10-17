#include "sound.h"
#include <QDebug>

Sound::Sound(QObject *parent) : QThread(parent),
    m_signal(0),
    m_phase(0),
    m_freq(0),
    m_time(0)
{
    start();
}

//Sound::~Sound()
//{
//    m_thread.quit();
//    m_thread.wait(2000);
//}

void Sound::beep(float freq_Hz, float duration_s)
{
    m_freq = freq_Hz;
//    float w = 2*M_PI*m_freq;
    if (m_freq > 0)
        m_time = lrintf(m_freq * duration_s) / m_freq;
    else
    {
        m_freq = 0;
        m_time = 0;
    }
    m_phase = 0;
    m_wait.wakeAll();
}

void Sound::requestInterruption()
{
    QThread::requestInterruption();
    m_wait.wakeAll();
}

void Sound::run()
{
    QAudioFormat format;
    format.setSampleRate(SAMPLE_FREQ);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    QAudioDeviceInfo info = QAudioDeviceInfo::defaultOutputDevice();
    if (!info.isFormatSupported(format))
    {
        qWarning() << "bad audio format, but pofig";
        format = info.nearestFormat(format);
    }

    m_audio = new QAudioOutput(format);
    m_audio->setVolume(0.5);
    m_audio->setBufferSize(SAMPLE_FREQ);
    m_device = m_audio->start();

    while (!isInterruptionRequested())
    {
        QByteArray ba;
        int Ncnt = lrintf(m_time * SAMPLE_FREQ);
        Ncnt = qMin(Ncnt, m_audio->periodSize());
        if (!Ncnt)
        {
            QMutex moo;
            moo.lock();
            m_wait.wait(&moo);
            moo.unlock();

            continue;
        }

//        qDebug() << m_audio->periodSize() << Ncnt << m_audio->bytesFree();
        float dt = 1.f / SAMPLE_FREQ;
        for (int i=0; i<Ncnt; i++)
        {
            m_phase += 2*M_PI*m_freq * dt;
            if (m_time > 0)
            {
                m_time -= dt;
                if (m_time < dt/2)
                {
                    m_time = 0;
//                    qDebug() << "phase" << m_phase << i << Ncnt;
                    emit beepFinished();
                }
            }
            while (m_phase > 2*M_PI)
                m_phase -= 2*M_PI;
            m_signal = sinf(m_phase);
            int16_t amp = m_time? 30000: 0;
            int16_t sample = m_signal * amp;
//            int16_t sample = m_signal > 0? amp: m_signal < 0? -amp: 0;
            ba.append(reinterpret_cast<const char*>(&sample), 2);
        }
        m_device->write(ba);

        int cnt = (m_audio->bufferSize() - m_audio->bytesFree()) / 2;
        msleep(cnt * 500 / SAMPLE_FREQ);
    }
}
