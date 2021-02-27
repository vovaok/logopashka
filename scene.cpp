#include "scene.h"

Scene::Scene() :
    m_v(0), m_w(0),
    m_vt(0), m_wt(0),
    m_wL(0), m_wR(0),
    m_phiL(0), m_phiR(0),
    m_x(0), m_y(0), m_phi(0),
    mPenEnabled(false),
    mBusy(false)
{
    setMinimumSize(256, 256);
    setBackColor(Qt::white);//QColor(240, 240, 240));
    setViewType(QPanel3D::fly);
    setAutoUpdate(false);
//    root()->showAxes(true);

    mMainCam = new Camera3D(this);
    mMainCam->setPosition(QVector3D(-60, 20, 20));
    mMainCam->setTarget(QVector3D(0, 0, 5));
    mMainCam->setTopDir(QVector3D(0, 0, 1));
    mMainCam->setDistanceLimit(500);
    mMainCam->setZoom(1);

    setCamera(mMainCam);

    mTopCam = new Camera3D(this);
    mTopCam->setPosition(QVector3D(0, 0, 100));
    mTopCam->setTarget(QVector3D(0, 0, 0));
    mTopCam->setTopDir(QVector3D(1, 0, 0));
    mTopCam->setDistanceLimit(500);
    mTopCam->setZoom(1);

    mFollowingCam = new Camera3D(this);
    mFollowingCam->setPosition(QVector3D(-50, 50, 30));
    mFollowingCam->setTarget(QVector3D(0, 0, 5));
    mFollowingCam->setTopDir(QVector3D(0, 0, 1));
    mFollowingCam->setDistanceLimit(500);
    mFollowingCam->setZoom(1);

//    setCamera(mFollowingCam);


    Light3D *fonarik = new Light3D(root());
    fonarik->setPosition(QVector3D(0, 0, 5000));
    fonarik->setConstantAtt(2);
//    fonarik->setLinearAtt(2e-4);
//    fonarik->setQuadraticAtt(1e-8);
    Light3D *l = new Light3D(fonarik);
    l->setDiffuseColor(QColor(255, 255, 224));
    l->setQuadraticAtt(1e-7);
    l->setPosition(QVector3D(-2000, 1000, 1000));
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

    mPlot = new DynamicTexture(this, QSize(sheet_resolution_px, sheet_resolution_px));
    drawCheckerboard(mPlot->paintBegin(), QRect(0, 0, sheet_resolution_px, sheet_resolution_px));
    mPlot->paintEnd();

    mFloor = new Primitive3D(root());
    mFloor->setPlane(QVector3D(100, 0, 0), QVector3D(0, 100, 0));
    mFloor->setDetalization(100, 100);
    mFloor->setColor(Qt::white);
    mFloor->setTexture(mPlot);
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

    mBase = new Mesh3D(root());
    mBase->loadModel(":/model/robot.wrl");
    mBase->setZPos(2.5f);
    mBase->setZOrient(180);
//    mBase->mesh()->rotate(90, QVector3D(0, -1, 0));
    //    mBase->updateModel();

    mWheelL = new Mesh3D(mBase);
    mWheelL->loadModel(":/model/wheel.wrl");
    mWheelL->setPosition(0, -5, 0);
    mWheelL->setXRot(90);

    mWheelR = new Mesh3D(mBase);
    mWheelR->loadModel(":/model/wheel.wrl");
    mWheelR->setPosition(0, 5, 0);
    mWheelR->setXRot(-90);

    mActu = new Mesh3D(mBase);
    mActu->loadModel(":/model/actuator.wrl");
    mActu->setPosition(8.5, 0, 0);

    mPen = new Primitive3D(mActu);
    mPen->setCylinder(0.4, 12);
    mPen->setColor(Qt::red);
    mPen->setPosition(-8.5, 0, -1.0);

    Primitive3D *pr = new Primitive3D(mPen);
    pr->setCone(0.2, 0.4, 1);
    pr->setColor(Qt::red);
    pr->setPosition(0, 0, -1.0);

    pr = new Primitive3D(pr);
    pr->setCylinder(0.1, 0.5);
    pr->setColor(Qt::darkRed);
    pr->setPosition(0, 0, -0.5);

    setPenEnabled(false);
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
            QColor col = c? QColor(255, 255, 255, 255): QColor(240, 240, 240, 255);
            p->fillRect(i, j, cellSize, cellSize, col);
        }
    }
}

void Scene::integrate(float dt)
{
    float b = 0.05; // m
    float r = 0.025; // m

    float oldx = m_x;
    float oldy = m_y;

    if (mBusy)
    {
        if (m_cmdTime >= dt)
            m_cmdTime -= dt;
        else if (m_cmdTime > 0)
        {
            m_vt = m_vt * m_cmdTime / dt;
            m_wt = m_wt * m_cmdTime / dt;
            m_cmdTime = 0;
        }
        else
        {
            m_cmdTime = 0;
            m_vt = m_wt = 0;
            mBusy = false;
            emit commandCompleted();
        }
    }

    m_wL = (m_wt * b - m_vt) / r;
    m_wR = (m_wt * b + m_vt) / r;

    m_phiL += m_wL * dt;
    m_phiR += m_wR * dt;

    m_v = (m_wR - m_wL) * r / 2;
    m_w = (m_wR + m_wL) * r / (2*b);

    m_phi += m_w * dt;
    m_x += m_v * cosf(m_phi) * dt;
    m_y += m_v * sinf(m_phi) * dt;

    mWheelL->setZRot(-m_phiL * 180 / M_PI);
    mWheelR->setZRot(-m_phiR * 180 / M_PI);

    setRobotPose(m_x, m_y, m_phi);

    if (mPenEnabled)
    {
        int c = sheet_resolution_px / 2;
        int z = sheet_resolution_px;
        QPainter *p = mPlot->paintBegin();
        p->setRenderHint(QPainter::Antialiasing);
        p->setPen(QPen(Qt::red, 3.0f, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        p->drawLine(c+1+oldx*z, c-oldy*z, c+1+m_x*z, c-m_y*z);
        mPlot->paintEnd();
    }

    Object3D *tob = mBase;
    QVector3D addPos = QVector3D(0, 0, 0);

    if (tob)
    {
        QVector3D fp = tob->pos() + addPos;
        float fi = tob->rot().z()*M_PI/180.0f;// tob->model()->yaw();
        float dd = 0.5; // distance to robot, m
        QVector3D fd = QVector3D(dd*100*cosf(fi), dd*100*sinf(fi), -25) ;
        QVector3D pp = fp - fd;
        static QVector3D ppold = QVector3D(0, 0, 0);
        if (ppold.isNull())
            ppold = pp;
        ppold = (63*ppold + pp) / 64;
        mFollowingCam->setPosition(ppold);
//        while (collisionCheck(mFollowingCam, 10))
//        {
//            ppold += (fp - ppold) * 0.5;
//            mFollowingCam->setPosition(ppold);
//        }
        mFollowingCam->setTarget(fp);
        mFollowingCam->setTopDir(QVector3D(0, 0, 1));
    }
}

void Scene::setControl(float v, float w)
{
    m_vt = v*0.05f;
    m_wt = w*0.25f;
}

void Scene::setRobotPose(float x, float y, float phi)
{
    mBase->setPosition(x*100, y*100, 2.5f);
    mBase->setZRot(phi * 180 / M_PI);
}

void Scene::setPenEnabled(bool enable)
{
    mPenEnabled = enable;
    mActu->setYRot(enable? 0: 5);
    m_cmdTime = 0.5f;
    mBusy = true;
}

void Scene::forward(float value)
{
    m_vt = vmax;
    m_wt = 0;
    m_cmdTime = value * 0.01f / vmax;
    mBusy = true;
}

void Scene::backward(float value)
{
    m_vt = -vmax;
    m_wt = 0;
    m_cmdTime = value * 0.01f / vmax;
    mBusy = true;
}

void Scene::left(float value)
{
    m_vt = 0;
    m_wt = wmax;
    m_cmdTime = value * M_PI / 180 / wmax;
    mBusy = true;
}

void Scene::right(float value)
{
    m_vt = 0;
    m_wt = -wmax;
    m_cmdTime = value * M_PI / 180 / wmax;
    mBusy = true;
}

void Scene::stop()
{
    m_vt = 0;
    m_wt = 0;
    m_cmdTime = 0;
    mBusy = true;
}

void Scene::reset()
{
    m_v = m_w = 0;
    m_vt = m_wt = 0;
    m_wL = m_wR = 0;
    m_phiL = m_phiR = 0;
    m_x = m_y = m_phi = 0;
    mPenEnabled = false;
    mBusy = false;
    m_cmdTime = 0;

    setRobotPose(0, 0, 0);
    mWheelL->setZRot(0);
    mWheelR->setZRot(0);

    drawCheckerboard(mPlot->paintBegin(), QRect(0, 0, sheet_resolution_px, sheet_resolution_px));
    mPlot->paintEnd();
}

void Scene::setView(Scene::ViewType viewtype)
{
    switch (viewtype)
    {
    case ViewMain: setCamera(mMainCam); break;
    case ViewTop: setCamera(mTopCam); break;
    case ViewFollow: setCamera(mFollowingCam); break;
    }
}


