#include "scene.h"

Scene::Scene()
{
    setMinimumSize(300, 300);
    setBackColor(Qt::white);
    setViewType(QPanel3D::ViewFly);
    setAutoUpdate(false);
    setMouseSensitivity(0.3);
//    root()->showAxes(true);

    m_mainCam = new Camera3D(this);
    m_mainCam->setPosition(QVector3D(60, 20, 20));
    m_mainCam->setTarget(QVector3D(0, 0, 5));
    m_mainCam->setTopDir(QVector3D(0, 0, 1));
    m_mainCam->setDistanceLimit(500);
    m_mainCam->setZoom(1);

    setCamera(m_mainCam);

    m_topCam = new Camera3D(this);
    m_topCam->setPosition(QVector3D(0, 0, 100));
    m_topCam->setTarget(QVector3D(0, 0, 0));
    m_topCam->setTopDir(QVector3D(1, 0, 0));
    m_topCam->setDistanceLimit(500);
    m_topCam->setZoom(1);
    m_topCam->lockDirection();

    m_followingCam = new Camera3D(this);
    m_followingCam->setPosition(QVector3D(60, 20, 20));
    m_followingCam->setTarget(QVector3D(0, 0, 5));
    m_followingCam->setTopDir(QVector3D(0, 0, 1));
    m_followingCam->setDistanceLimit(500);
    m_followingCam->setZoom(1);
    m_followingCam->setFollowing(true);

    m_chasingCam = new Camera3D(this);
    m_chasingCam->setPosition(QVector3D(-50, 0, 20));
    m_chasingCam->setTarget(QVector3D(0, 0, 5));
    m_chasingCam->setTopDir(QVector3D(0, 0, 1));
    m_chasingCam->setDistanceLimit(500);
    m_chasingCam->setZoom(1);
    m_chasingCam->setFollowing(true);
    m_chasingCamDistance = 50; // cm

//    setCamera(mFollowingCam);


    Light3D *fonarik = new Light3D(root());
    fonarik->setPosition(QVector3D(0, 0, 5000));
    fonarik->setConstantAtt(2);
//    fonarik->setLinearAtt(2e-4);
//    fonarik->setQuadraticAtt(1e-8);
    Light3D *l = new Light3D(fonarik);
    l->setDiffuseColor(QColor(255, 255, 224));
    l->setQuadraticAtt(1e-7);
    l->setPosition(QVector3D(2000, 1000, 1000));
    l = new Light3D(fonarik);
    l->setDiffuseColor(QColor(255, 255, 230));
    l->setSpecularColor(QColor(240, 240, 255));
    l->setQuadraticAtt(2e-8);
    l->setPosition(QVector3D(-3000, 5000, 2000));

//    // floor texture:
//    QImage img(1000, 1000, QImage::Format_ARGB32_Premultiplied);
//    QPainter p(&img);
//    drawCheckerboard(&p, img.rect());
//    p.end();

    m_plot = new DynamicTexture(this, QSize(sheet_resolution_px, sheet_resolution_px));
    drawCheckerboard(m_plot->paintBegin(), QRect(0, 0, sheet_resolution_px, sheet_resolution_px));
    m_plot->paintEnd();

    m_floor = new Primitive3D(root());
    m_floor->setPlane(QVector3D(100, 0, 0), QVector3D(0, 100, 0));
    m_floor->setDetalization(100, 100);
    m_floor->setColor(Qt::white);
    m_floor->setTexture(m_plot);
//    mFloor->setTexture(new StaticTexture(this, img));

//    Primitive3D *jopa = new Primitive3D(root());
//    jopa->setCylinder(0.5, 10);
//    jopa->setXRot(90);

//    Primitive3D *sheet = new Primitive3D(mFloor);
//    sheet->setPlane(QVector3D(100, 0, 0), QVector3D(0, 100, 0));
//    sheet->setDetalization(100, 100);
//    sheet->setColor(Qt::white);
//    sheet->setTexture(mPlot);
//    sheet->setZPos(0.1f);

    m_robot = new RobotModel(root());
    connect(m_robot, &RobotModel::needClearScreen, this, &Scene::reset);
}

void Scene::drawCheckerboard(QPainter *p, const QRect &rect)
{
    int imgcx = rect.width() / 2;
    int imgcy = rect.height() / 2;
    p->setPen(Qt::NoPen);
    float cellSize = 0.05f * rect.width();
    int cellStartX = imgcx - (imgcx / cellSize + 1) * cellSize;
    int cellStartY = imgcy - (imgcy / cellSize + 1) * cellSize;
    for (float i=cellStartX; i<rect.width(); i+=cellSize)
    {
        for (float j=cellStartY; j<rect.height(); j+=cellSize)
        {
            bool c = lrintf((i + j) / cellSize) & 1;
            int v = 240 + m_clear;
            QColor col = c? QColor(255, 255, 255, 255): QColor(v, v, v, 255);
            p->fillRect(i, j, cellSize, cellSize, col);
        }
    }
}

void Scene::integrate(float dt)
{
    m_robot->integrate(dt);

    if (m_clear)
    {
        --m_clear;
        drawCheckerboard(m_plot->paintBegin(), QRect(0, 0, sheet_resolution_px, sheet_resolution_px));
        m_plot->paintEnd();
    }

    if (m_robot->penState())
    {
        int c = sheet_resolution_px / 2;
        int z = sheet_resolution_px;
        QPainter *p = m_plot->paintBegin();
        p->setWindow(-50, 50, 100, -100);
        p->setRenderHint(QPainter::Antialiasing);
        m_robot->updateSheet(p);
        m_plot->paintEnd();
    }

    Object3D *tob = m_robot;

    if (tob)
    {
        QVector3D fp = tob->pos();
        m_followingCam->setTarget(fp);
        QVector3D fdir = m_followingCam->position() - fp;
        float fdist = fdir.length();
        if (fdist < 20.0f)
            m_followingCam->setPosition(fp + fdir * (20.0f / fdist));
        if (fdist > 100.0f)
            m_followingCam->setPosition(fp + fdir * (100.0f / fdist));

        QVector3D ppold = m_chasingCam->position();
        float hei = 25; // POV height
        float fi = tob->rot().z()*M_PI/180.0f;
        if (ppold != m_chaseCamPos)
            m_chasingCamDistance = (tob->pos() - ppold + QVector3D(0, 0, hei)).length(); // distance to the target, cm
        m_chasingCamDistance = qBound(10.0f, m_chasingCamDistance, 100.0f);
        QVector3D fd = QVector3D(m_chasingCamDistance * cosf(fi), m_chasingCamDistance * sinf(fi), -hei) ;
        QVector3D pp = fp - fd;
        m_chaseCamPos = (31*ppold + pp) / 32;
        m_chasingCam->setPosition(m_chaseCamPos);
//        while (collisionCheck(mFollowingCam, 10))
//        {
//            ppold += (fp - ppold) * 0.5;
//            mFollowingCam->setPosition(ppold);
//        }
        m_chasingCam->setTarget(fp);
    }
}

Scene::ViewType Scene::view()
{
    if (camera() == m_mainCam)
        return ViewMain;
    if (camera() == m_topCam)
        return ViewTop;
    if (camera() == m_followingCam)
        return ViewFollow;
    if (camera() == m_chasingCam)
        return ViewChase;
}

void Scene::reset()
{
    m_robot->setPose(0, 0, 0);
    m_clear = 15;
}

void Scene::setView(Scene::ViewType viewtype)
{
    switch (viewtype)
    {
    case ViewMain: setCamera(m_mainCam); break;
    case ViewTop: setCamera(m_topCam); break;
    case ViewFollow: setCamera(m_followingCam); break;
    case ViewChase: setCamera(m_chasingCam); break;
    }
}


