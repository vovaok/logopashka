#ifndef ROBOT_H
#define ROBOT_H

#include "objnetdevice.h"

using namespace Objnet;

class Robot : public ObjnetDevice
{
    Q_OBJECT

public:

private:
    bool mValid;
    bool mEnabled;
    int mBusyV, mBusyW;
    float mVt, mWt;
    float mV, mW;

private slots:
    void onReady();
    void onObjectReceived(QString name, QVariant);

public:
    Robot();

    void setControl(float v, float w);

    void forward(float value);
    void backward(float value);
    void left(float value);
    void right(float value);
    void stop();

    bool isEnabled() const {return mEnabled;}
    bool isBusy() const {return mBusyV || mBusyW;}

signals:
    void commandCompleted();

public slots:
    void update();
    void setEnabled(bool enabled);
//    void clearFault();
};

#endif // ROBOCOMCTRL_H
