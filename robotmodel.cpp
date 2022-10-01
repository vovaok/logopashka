#include "robotmodel.h"

RobotModel::RobotModel(Object3D *parent) :
    Object3D(parent),
    m_v(0), m_w(0),
    m_vt(0), m_wt(0),
    m_wL(0), m_wR(0),
    m_phiL(0), m_phiR(0),
    m_x(0), m_y(0), m_phi(0),
    m_oldx(0), m_oldy(0),
    m_penEnabled(false)
{
    m_busy = false;

    blinking = 0;
    sleepy = 0;//7;
    eyelid = 0;
    eyeSize = 4;
    size = 2;
    eyeX = 0;
    eyeY = 0;
    mood = 2;
    joy = 0;

    mBase = new Mesh3D(this);
    mBase->loadModel(":/model/robot.wrl");
//    mBase->setZPos(2.5f);
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

    Primitive3D *face = new Primitive3D(mBase);
    face->setPlane(QVector3D(3.5f, 0, 0), QVector3D(0, 7, 0));
    face->setOrient(0, -80, 0);
    face->setPosition(-1.8, 0, 2);
    face->setColor(Qt::white, Qt::white, Qt::white);
    mFace = new DynamicTexture(scene(), QSize(10, 18));
    face->setTexture(mFace);

    updateFace();
}

void RobotModel::forward(float value)
{
    m_vt = vmax;
    m_wt = 0;
    m_cmdTime = value * 0.01f / vmax;
    m_busy = true;
}

void RobotModel::backward(float value)
{
    m_vt = -vmax;
    m_wt = 0;
    m_cmdTime = value * 0.01f / vmax;
    m_busy = true;
}

void RobotModel::right(float value)
{
    m_vt = 0;
    m_wt = -wmax;
    m_cmdTime = value * M_PI / 180 / wmax;
    m_busy = true;
}

void RobotModel::left(float value)
{
    m_vt = 0;
    m_wt = wmax;
    m_cmdTime = value * M_PI / 180 / wmax;
    m_busy = true;
}

void RobotModel::penUp()
{
    setPenEnabled(false);
}

void RobotModel::penDown()
{
    setPenEnabled(true);
}

void RobotModel::clearScreen()
{
    reset();
    emit needClearScreen();
}

void RobotModel::stop()
{
    m_vt = 0;
    m_wt = 0;
    m_cmdTime = 0;
    m_busy = true;
}

void RobotModel::integrate(float dt)
{
    const float b = 0.05; // m
    const float r = 0.025; // m

    m_oldx = m_x;
    m_oldy = m_y;

    if (m_busy)
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
            m_busy = false;
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

    setPose(m_x, m_y, m_phi);
    updateFace();
}

void RobotModel::setControl(float v, float w)
{
    m_vt = v*0.05f;
    m_wt = w*0.25f;
}

void RobotModel::setPose(float x, float y, float phi)
{
    setPosition(x*100, y*100, 2.5f);
    setZRot(phi * 180 / M_PI);
}

void RobotModel::setPenEnabled(bool enable)
{
    m_penEnabled = enable;
    mActu->setYRot(enable? 0: 5);
    m_cmdTime = 0.1f;
    m_busy = true;
}

void RobotModel::reset()
{
    m_v = m_w = 0;
    m_vt = m_wt = 0;
    m_wL = m_wR = 0;
    m_phiL = m_phiR = 0;
    m_x = m_y = m_phi = 0;
    m_penEnabled = false;
    m_cmdTime = 0;
    mWheelL->setZRot(0);
    mWheelR->setZRot(0);
    m_busy = false;
}

void RobotModel::updateFace()
{
    uint32_t backColor = m_eyeColor & 0x00FFFFFF;
    QImage faceImage(10, 18, QImage::Format_ARGB32_Premultiplied);
    faceImage.fill(backColor);

    blinking++;
    if (blinking > 200)
        blinking = 0;

    // draw
    if (rand() < RAND_MAX / 100)
    {
        eyeX = (rand() % lrintf(5-sleepy/2)) - 2;
        eyeY = (rand() % lrintf(6-sleepy)) - 2;
    }

    if (rand() < RAND_MAX / 500)
    {
        if (rand() & 1)
            mood++;
        else
            mood--;
        if (mood > 6)
            mood = 6;
        else if (mood < -6)
            mood = -6;
    }

    if (eyelid < sleepy)
         eyelid = sleepy;

    int R = eyeSize;
    int R2 = R*R;
    float r = size * 0.5f;
    float r2 = r*r;
    float ex = (eyeX * (2*R)) / 10.0f;
    float ey = (-eyeY * (2*R)) / 10.0f;

    for (int i=0; i<8; i++)
    {
        float x = i - 3.5f;
        for (int j=0; j<8; j++)
        {
//            float v = 1.0f;
            float y = j - 3.5f;
            faceImage.setPixel(8-j, i+1, backColor);
            faceImage.setPixel(8-j, 16-i, backColor);
//            screen->pixel(i, j) = 0;
//            screen->pixel(15-i, j) = 0;
            if (j < eyelid)
                continue;
            if (x*mood + 4*(y+R-eyelid) < 0)
                continue;
            if ((y-R+1) > (1-joy))
                continue;
            float Ro2 = x*x*1.42f + y*y;
            if (Ro2 <= R2)
            {
                faceImage.setPixel(8-j, i+1, m_eyeColor);
                faceImage.setPixel(8-j, 16-i, m_eyeColor);
//                screen->pixel(i, j) = 1;
//                screen->pixel(15-i, j) = 1;
            }
        }
    }

    int ri = floorf(r);
    for (int i=0; i<ri; i++)
    {
        int rj = lrintf(r2 / ri);
        for (int j=0; j<rj; j++)
        {
            int i0 = i - ri/2;
            int j0 = j - rj/2;
            int x = lrintf(i0 + 4 + ex);
            int y = lrintf(j0 + 4 + ey);
            int r02 = (i0+0.5f)*(i0+0.5f) + (j0+0.5f)*(j0);
            if (r02 >= r2/4)
                continue;
            if (x < 0 || x >= 8)
                continue;
            if (y < 0 || y >= 8)
                continue;
            faceImage.setPixel(8-y, x+1, backColor);
            faceImage.setPixel(8-y, x+9, backColor);
//            screen->pixel(x, y) = 0;
//            screen->pixel(x+8, y) = 0;
        }
    }

    //unsigned char mask = 0xFF;
    if (blinking < 8)
    {
        eyelid = blinking;
        //mask = ~((1 << morganie) - 1);
    }
    else if (blinking < 16)
    {
        eyelid = 15 - blinking;
        //mask = ~((1 << (15 - morganie)) - 1);
    }

    QPainter *p = mFace->paintBegin();
    p->setCompositionMode(QPainter::CompositionMode_Source);
    p->fillRect(faceImage.rect(), Qt::transparent);
    p->setCompositionMode(QPainter::CompositionMode_SourceOver);
    p->drawImage(0, 0, faceImage);
    mFace->paintEnd();
}

void RobotModel::updateSheet(QPainter *p)
{
    if (m_penEnabled)
    {
        float pen_width = 0.3f * expf(-m_v*m_v*50); // max 0.3 cm
        p->setPen(QPen(Qt::red, pen_width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        if (m_v)
            p->drawLine(QLineF(m_oldx*100, m_oldy*100, m_x*100, m_y*100));
        else
            p->drawPoint(QPointF(m_x*100, m_y*100));

    }
}
