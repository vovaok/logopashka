#ifndef SPACEMOUSE_H
#define SPACEMOUSE_H

#include <QObject>
#include "usbhid.h"

class SpaceMouse : public QObject
{
    Q_OBJECT

public:
    struct AxesValues
    {
        float x, y, z;
        float rx, ry, rz;
    };

private:
    struct Joy3DReport
    {
        short x, y, z;
        short rx, ry, rz;
    };
    UsbHid *mDevice;
    Joy3DReport mAxisData;
    unsigned char mButtons;
    unsigned char mBattery;

private slots:

public:
    explicit SpaceMouse(QObject *parent = nullptr);

    bool isPresent() const {return mDevice->isOpen();}
    bool isButtonPressed(int button) const;
    int batteryLevel() const {return mBattery;}
    AxesValues getAxesValues();

signals:

public slots:
    void read();
};

#endif // SPACEMOUSE_H
