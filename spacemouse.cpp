#include "spacemouse.h"

SpaceMouse::SpaceMouse(QObject *parent) : QObject(parent),
    mDevice(0L),
    mButtons(0),
    mBattery(0)
{
    memset(&mAxisData, 0, sizeof(Joy3DReport));
    mDevice = new UsbHid(0x256F, 0xC652/*0xC62E*/, this);
//    joy3D = new UsbHid(0x256F, 0xC62E, this);
//    QTimer *joy3dtimer = new QTimer(joy3D);
//    connect(joy3dtimer, SIGNAL(timeout()), SLOT(onJoy3DTimer()));
//    joy3dtimer->start(16);
//    joy3D->setPollingInterval(10);
//    joy3D->setCurrentReportId(1, 12);
//    connect(joy3D, SIGNAL(reportReceived(QByteArray)), SLOT(onJoy3DHidReport(QByteArray)));
//    joy3D->start(QThread::NormalPriority);

//    QTimer *timer = new QTimer(this);
//    connect(timer, SIGNAL(timeout()), SLOT(onTimer()));
//    timer->start(16);
}

void SpaceMouse::read()
{
    if (mDevice->isOpen())
    {
        QByteArray ba;
        do
        {
            ba = mDevice->read(13);
            if (ba.size())
            {
                unsigned char reportId = ba[0];
                if (reportId == 1)
                {
                    mAxisData = *reinterpret_cast<const Joy3DReport*>(ba.data() + 1);
                }
                else if (reportId == 3)
                {
                    mButtons = ba[1];
                }
                else if (reportId == 23)
                {
                    mBattery = ba[1];
                }
            }
        } while (ba.size());
    }
    else if (!mDevice->availableDevices().isEmpty())
    {
        QStringList list = mDevice->availableDevices();
        foreach (QString path, list)
        {
            if (path.contains("col02"))
            {
                mDevice->setDevice(path);
                break;
            }
        }
        mDevice->open();
    }
    else
    {
        mDevice->close();
    }
}

bool SpaceMouse::isButtonPressed(int button) const
{
    return mButtons & (1 << button);
}

SpaceMouse::AxesValues SpaceMouse::getAxesValues()
{
    AxesValues v;
    v.x = mAxisData.x / 32768.0f;
    v.y = mAxisData.y / 32768.0f;
    v.z = mAxisData.z / 32768.0f;
    v.rx = mAxisData.rx / 32768.0f;
    v.ry = mAxisData.ry / 32768.0f;
    v.rz = mAxisData.rz / 32768.0f;
    memset(&mAxisData, 0, sizeof(Joy3DReport));
    return v;
}
