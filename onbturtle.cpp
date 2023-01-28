#include "onbturtle.h"

OnbTurtle::OnbTurtle()
{
    mValid = false;
    mEnabled = false;
    mBusyV = mBusyW = mBusyPen = 0;
    mVt = mWt = 0;
    mV = mW = 0;
    m_penState = false;
    m_busy = false;

    connect(this, SIGNAL(ready()), SLOT(onReady()));
    //! @todo clear mValid on disconnect
    connect(this, SIGNAL(objectReceived(QString,QVariant)), SLOT(onObjectReceived(QString,QVariant)));
    m_timer = new QTimer(this);
    m_timer->setInterval(33);
    connect(m_timer, &QTimer::timeout, this, &OnbTurtle::update);
}

void OnbTurtle::onReady()
{
    bindVariable("isStarted", mEnabled);
    bindVariable("v*", mVt);
    bindVariable("w*", mWt);
    bindVariable("v", mV);
    bindVariable("w", mW);

    requestObject("isStarted");

    m_timer->start();
}

void OnbTurtle::onObjectReceived(QString name, QVariant)
{
    mValid = true;
    if (name == "v*")
    {
        if (!mVt && mBusyV)
            --mBusyV;
    }
    else if (name == "w*")
    {
        if (!mWt && mBusyW)
            --mBusyW;
    }

    if (mBusyPen)
    {
        --mBusyPen;
    }

    m_busy = mBusyV || mBusyW || mBusyPen;
}

void OnbTurtle::update()
{
    QStringList req;
    req << "v*" << "w*" << "v" << "w";
    groupedRequest(req.toVector().toStdVector());
}

void OnbTurtle::setEnabled(bool enabled)
{
    if (enabled)
        requestObject("enable");
    else
        requestObject("disable");
}

void OnbTurtle::setControl(float v, float w)
{
    mVt = v;
    mWt = w;
    sendObject("v*");
    sendObject("w*");
}

void OnbTurtle::forward(float value)
{
    mBusyV = 1;
    m_busy = true;
    sendObject("forward", value*0.5*20/44);
}

void OnbTurtle::backward(float value)
{
    mBusyV = 1;
    m_busy = true;
    sendObject("backward", value*0.5*20/44);
}

void OnbTurtle::left(float value)
{
    mBusyW = 1;
    m_busy = true;
    sendObject("left", value * 7.3f / 360);
}

void OnbTurtle::right(float value)
{
    mBusyW = 1;
    m_busy = true;
    sendObject("right", value * 7.3f / 360);
}

void OnbTurtle::stop()
{
    mVt = 0;
    mWt = 0;
    m_busy = false;
    sendObject("stop");
}

void OnbTurtle::penUp()
{
    mBusyPen = 1;
    m_busy = true;
    m_penState = false;
    sendObject("penUp");
}

void OnbTurtle::penDown()
{
    mBusyPen = 1;
    m_busy = true;
    m_penState = true;
    sendObject("penDown");
}

bool OnbTurtle::penState() const
{
    return m_penState;
}

