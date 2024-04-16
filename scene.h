#ifndef SCENE_H
#define SCENE_H

#include "panel3d.h"
#include "robotmodel.h"

class Scene : public QPanel3D
{
    Q_OBJECT

public:
    typedef enum
    {
        ViewMain,
        ViewTop,
        ViewFollow,
        ViewChase
    } ViewType;

private:
    Camera3D *m_mainCam, *m_topCam, *m_followingCam, *m_chasingCam;
    DynamicTexture *m_plot;
    Primitive3D *m_floor;
    RobotModel *m_robot;

    QVector3D m_chaseCamPos;
    float m_chasingCamDistance;

    const int sheet_resolution_px = 4000;
    int m_clear = 15;

    void drawCheckerboard(QPainter *p, const QRect &rect);

public:
    Scene();

    void integrate(float dt);

    RobotModel *turtle() {return m_robot;}

    ViewType view();

//signals:
//    void commandCompleted();

public slots:
    void setView(ViewType viewtype);
    void reset();

    void onTurtleCommand(QString s, QString arg);
};

#endif // SCENE_H
