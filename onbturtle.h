#ifndef ONBTURTLE_H
#define ONBTURTLE_H

#include "objnetdevice.h"
#include "turtleinterface.h"
#include <QObject>

using namespace Objnet;

class OnbTurtle : public ObjnetDevice, public TurtleInterface
{
    Q_OBJECT
public:
    OnbTurtle();

    virtual void forward(float value) override;
    virtual void backward(float value) override;
    virtual void right(float value) override;
    virtual void left(float value) override;
    virtual void penUp() override;
    virtual void penDown() override;
    virtual bool penState() const override;

    // optional procedures:
    virtual void stop() override;
//    virtual void clearScreen() override;
////    virtual void move(float x, float y) override;
//    virtual void setColor(unsigned int rgb) override;
//    virtual void arc(float radius, float degrees) override;
//    virtual void print(const char *s) override; // print on turtle's screen
//    virtual void cls() override; // clear turtle's screen
//    virtual void showError(const char *message) override;
//    virtual void sound(float freq, float duration) override;

//    virtual void setProperty(const char *name, float value) override;
//    virtual float getProperty(const char *name) const override;
//    virtual bool hasProperty(const char *name) const override;

    virtual void setControl(float v, float w) override;

    bool isEnabled() const {return mEnabled;}

public slots:
    void setEnabled(bool enabled);

private:
    bool mValid;
    bool mEnabled;
    int mBusyV, mBusyW, mBusyPen;
    float mVt, mWt;
    float mV, mW;
    bool m_penState;
    QTimer *m_timer;

private slots:
    void onReady();
    void onObjectReceived(QString name, QVariant);
    void update();
};

#endif // ONBTURTLE_H
