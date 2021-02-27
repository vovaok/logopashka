#ifndef SCENE_H
#define SCENE_H

#include "panel3d.h"

class Scene : public QPanel3D
{
    Q_OBJECT

public:
    typedef enum
    {
        ViewMain,
        ViewTop,
        ViewFollow
    } ViewType;

private:
    Camera3D *mMainCam, *mTopCam, *mFollowingCam;
    Primitive3D *mFloor;
    Mesh3D *mBase;
    Mesh3D *mWheelL, *mWheelR;
    Mesh3D *mActu;
    Primitive3D *mPen;
    DynamicTexture *mPlot;

    const int sheet_resolution_px = 2000;
    const float vmax = 0.1; // m/s
    const float wmax = 1.57; // rad/s

    float m_v, m_w;
    float m_vt, m_wt;
    float m_wL, m_wR;
    float m_phiL, m_phiR;
    float m_x, m_y, m_phi;

    bool mPenEnabled;
    bool mBusy;
    float m_cmdTime;

    void drawCheckerboard(QPainter *p, const QRect &rect);

public:
    Scene();

    void integrate(float dt);

    void setControl(float v, float w);
    void setRobotPose(float x, float y, float phi);
    void setPenEnabled(bool enable);

    void forward(float value); // cm
    void backward(float value); // cm
    void left(float value); // deg
    void right(float value); // deg

signals:
    void commandCompleted();

public slots:
    void penDown() {setPenEnabled(true);}
    void penUp() {setPenEnabled(false);}
    void stop();

    void reset();

    void setView(ViewType viewtype);
};

#endif // SCENE_H
