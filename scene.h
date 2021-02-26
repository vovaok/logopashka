#ifndef SCENE_H
#define SCENE_H

#include "panel3d.h"

class Scene : public QPanel3D
{
    Q_OBJECT

private:
    Camera3D *mMainCam, *mFollowingCam;
    Primitive3D *mFloor;
    Mesh3D *mBase;
    Mesh3D *mWheelL, *mWheelR;
    Mesh3D *mActu;
    Primitive3D *mPen;
    DynamicTexture *mPlot;

    float m_v, m_w;
    float m_vt, m_wt;
    float m_wL, m_wR;
    float m_phiL, m_phiR;
    float m_x, m_y, m_phi;

    bool mPenEnabled;

public:
    Scene();

    void integrate(float dt);

    void setControl(float v, float w);
    void setRobotPose(float x, float y, float phi);
    void setPenEnabled(bool enable);

public slots:
    void penDown() {setPenEnabled(true);}
    void penUp() {setPenEnabled(false);}
};

#endif // SCENE_H
