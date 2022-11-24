#ifndef ROBOTMODEL_H
#define ROBOTMODEL_H

#include "turtleinterface.h"
#include "object3d.h"
#include "mesh3d.h"
#include "primitive3d.h"
#include "texture.h"
#include "sound.h"

class RobotModel : public Object3D, public TurtleInterface
{
    Q_OBJECT

public:
    RobotModel(Object3D *parent);
    ~RobotModel();

    virtual void forward(float value) override;
    virtual void backward(float value) override;
    virtual void right(float value) override;
    virtual void left(float value) override;
    virtual void penUp() override;
    virtual void penDown() override;
    virtual void arc(float radius, float degrees) override;
    virtual void clearScreen() override;
    void stop() override;
    virtual void setColor(unsigned int rgb) override;
    void putchar(char c);
    virtual void print(const char *s) override;
    virtual void cls() override;
    virtual void showError(const char *message) override;
    virtual void sound(float freq, float dur);

    virtual void setControl(float v, float w) override;

    virtual void setProperty(const char *name, float value) override;
    virtual float getProperty(const char *name) const override;
    virtual bool hasProperty(const char *name) const override;

    void integrate(float dt);

    void setPose(float x, float y, float phi);
    void setPenEnabled(bool enable);
    virtual bool penState() const override {return m_penEnabled;}
//    float speed() const {return m_v;}

    void setBalloonColor(QColor color);

signals:
    void needClearScreen();

protected:
    friend class Scene;
    void updateSheet(QPainter *p);

private:
    Mesh3D *mBase;
    Mesh3D *mWheelL, *mWheelR;
    Mesh3D *mActu;
    Primitive3D *mPen;
    DynamicTexture *mFace;
    DynamicTexture *mScreen;
    Primitive3D *mBalloon, *mBalloonArrow;

    Sound *m_sound;

    const uint32_t m_eyeColor = 0xFF40A0FF;
//    uint8_t eyes[16]; // two 8x8 LED matrices
    int blinking;
    float sleepy;
    int eyelid;
    int eyeSize;
    int size;
    int eyeX;
    int eyeY;
    int mood;
    int joy;

    const float vmax = 0.1; // m/s
    const float wmax = 1.57; // rad/s

    float m_v, m_w;
    float m_vt, m_wt;
    float m_wL, m_wR;
    float m_phiL, m_phiR;
    float m_x, m_y, m_phi;
    float m_oldx, m_oldy;

    QColor m_penColor;
    bool m_penEnabled;
    float m_cmdTime;

    QImage m_screenImg;
    bool m_needScreenUpdate;
    int m_screenX, m_screenY; // turtle's screen text coords
    float m_balloonScale, m_balloonScaleRate;

    void reset();
    void updateFace();
    void setPenColor(QColor color);
    void setBalloonVisible(bool visible);
    void updateScreen();
};

#endif // ROBOTMODEL_H
