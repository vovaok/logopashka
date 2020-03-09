#ifndef SCENE_H
#define SCENE_H

#include "panel3d.h"

class Scene : public QPanel3D
{
    Q_OBJECT

private:
    Camera3D *mMainCam;
    Primitive3D *mFloor;
    Mesh3D *mBase;
    Mesh3D *mLink[6];
    Primitive3D *mTest;

public:
    Scene();

    void setAngles(float *q);
    void setTestPos(QVector3D p);
};

#endif // SCENE_H
