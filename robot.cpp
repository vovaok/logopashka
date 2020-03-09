#include "robot.h"

Robot::Robot()
{
    mValid = false;
    mEnabled = false;
    mBusyV = mBusyW = 0;
    mVt = mWt = 0;
    mV = mW = 0;

    connect(this, SIGNAL(ready()), SLOT(onReady()));
    //! @todo clear mValid on disconnect
    connect(this, SIGNAL(objectReceived(QString,QVariant)), SLOT(onObjectReceived(QString,QVariant)));
}

void Robot::onReady()
{
    bindVariable("isStarted", mEnabled);
    bindVariable("v*", mVt);
    bindVariable("w*", mWt);
    bindVariable("v", mV);
    bindVariable("w", mW);

    requestObject("isStarted");
}

void Robot::onObjectReceived(QString name, QVariant)
{
//    if (name == "q_real")
        mValid = true;
    if (name == "v*")
    {
        if (!mVt && mBusyV)
        {
            --mBusyV;
            if (!mBusyV)
                emit commandCompleted();
        }
    }
    else if (name == "w*")
    {
        if (!mWt && mBusyW)
        {
            --mBusyW;
            if (!mBusyW)
                emit commandCompleted();
        }
    }
}

void Robot::update()
{
    QStringList req;
    req << "v*" << "w*" << "v" << "w";
    groupedRequest(req.toVector().toStdVector());
}

void Robot::setEnabled(bool enabled)
{
    if (enabled)
        requestObject("enable");
    else
        requestObject("disable");
}

void Robot::setControl(float v, float w)
{
    mVt = v;
    mWt = w;
    sendObject("v*");
    sendObject("w*");
}

void Robot::forward(float value)
{
    mBusyV = 5;
    sendObject("forward", value*0.5);
}

void Robot::backward(float value)
{
    mBusyV = 5;
    sendObject("backward", value*0.5);
}

void Robot::left(float value)
{
    mBusyW = 5;
    sendObject("right", value * 6.8f / 360);
}

void Robot::right(float value)
{
    mBusyW = 5;
    sendObject("left", value * 6.8f / 360);
}

void Robot::stop()
{
    mVt = 0;
    mWt = 0;
    sendObject("stop");
}
