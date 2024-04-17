#include "robotmodel.h"
#include "panel3d.h"

extern const uint8_t Font8x8_Table[];

RobotModel::RobotModel(Object3D *parent) :
    Object3D(parent),
    m_v(0), m_w(0),
    m_vt(0), m_wt(0),
    m_wL(0), m_wR(0),
    m_phiL(0), m_phiR(0),
    m_x(0), m_y(0), m_phi(0),
    m_oldx(0), m_oldy(0),
    m_penColor(Qt::red),
    m_penEnabled(false)
{
    m_busy = false;
    m_needScreenUpdate = false;

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
//    mPen->setColor(Qt::red);
    mPen->setPosition(-8.5, 0, -1.0);

    Primitive3D *pr = new Primitive3D(mPen);
    pr->setCone(0.2, 0.4, 1);
//    pr->setColor(Qt::red);
    pr->setPosition(0, 0, -1.0);

    pr = new Primitive3D(pr);
    pr->setCylinder(0.1, 0.5);
//    pr->setColor(Qt::darkRed);
    pr->setPosition(0, 0, -0.5);

    setPenColor(m_penColor);
    setPenEnabled(false);

    m_star = new Mesh3D(mPen);
    m_star->loadModel(":/model/star.wrl", Mesh3D::AssignOneColor);
    m_star->mesh()->scaleUniform(10);
    m_star->updateModel();
    m_star->setColor(QColor(255, 128, 0));
    m_star->setPosition(0, 0, 11);
    m_star->setZOrient(90);
//    m_star->setVisible(false);
    m_star->setUniformScale(0);

    m_magic = new PointCloud3D(scene()->root());
    m_magic->setPointSize(5);
    m_magic->setVisible(false);
    for (int i=0; i<100; i++)
    {
        QColor c = QColor::fromHsl(rand() % 360, 255, 128);
        magic << MagicPoint{0, 0, 0, c.rgb(), 0, 0, 0, 0};
    }
    m_magic->setDataBuffer(magic.data(), 12, sizeof(MagicPoint), magic.size());

    Primitive3D *face = new Primitive3D(mBase);
    face->setPlane(QVector3D(3.5f, 0, 0), QVector3D(0, 7, 0));
    face->setOrient(0, -80, 0);
    face->setPosition(-1.8, 0, 2);
    face->setColor(Qt::white, Qt::white, Qt::white);
    mFace = new DynamicTexture(scene(), QSize(10, 18));
    face->setTexture(mFace);

    mBalloon = new Primitive3D(mBase);
    mBalloon->setRoundedRect(12, 6, 0.5);
    mBalloon->setYOrient(180);
//    mBalloon->setPlane(QVector3D(10, 0, 0), QVector3D(0, -5, 0));
//    QColor balcol(0, 255, 255, 144);
//    mBalloon->setColor(balcol);
    mBalloon->setCenter(0, 5, 0);
    mBalloon->setPosition(-2, 0, 5);
    mBalloon->setDepthTestDisabled(true);

    mScreen = new DynamicTexture(scene(), QSize(128+12, 64+12)); // 6px borders
    m_screenImg = QImage(128, 64, QImage::Format_ARGB32_Premultiplied);
//    QImage img(256+24, 128+24, QImage::Format_ARGB32_Premultiplied);
//    img.fill(Qt::white);
//    for (int i=0; i<255; i++)
//    {
//        for (int j=0; j<128; j++)
//        {
//            uint32_t c = 0xFF000000 | i | (j<<16);
//            if ((i^j) & 8)
//                c = 0xFFFFFFFF;
//            img.setPixel(i+12, j+12, c);
//        }
//    }
    mBalloon->setTexture(mScreen);

    mBalloonArrow = new Primitive3D(mBalloon);
//    mBalloonArrow->setColor(balcol);
    QPolygonF poly;
    poly << QPointF(0, -2);
    poly << QPointF(1, 0);
    poly << QPointF(-1, 0);
    mBalloonArrow->setPolygon(poly);
    mBalloonArrow->setPosition(0, -3, 0);

    cls();
    setBalloonColor(Qt::cyan);
    m_balloonScale = 0;
    m_balloonScaleRate = 0;
    mBalloon->setVisible(false);
    mBalloonArrow->setVisible(false);

    updateFace();

    m_sound = new Sound(this);
}

RobotModel::~RobotModel()
{
    m_sound->requestInterruption();
    m_sound->quit();
    m_sound->wait(1000);
    m_sound->terminate();
}

void RobotModel::forward(float value)
{
    m_vt = vmax;
    m_wt = 0;
    m_cmdTime = value * 0.01f / vmax;
    m_busy = true;
    if (m_tracePath)
        m_pathTraced << "tr " + QString::number(value);
}

void RobotModel::backward(float value)
{
    m_vt = -vmax;
    m_wt = 0;
    m_cmdTime = value * 0.01f / vmax;
    m_busy = true;
    if (m_tracePath)
        m_pathTraced << "tr " + QString::number(-value);
}

void RobotModel::right(float value)
{
    m_vt = 0;
    m_wt = -wmax;
    m_cmdTime = value * M_PI / 180 / wmax;
    m_busy = true;
    if (m_tracePath)
        m_pathTraced << "rot " + QString::number(-value);
}

void RobotModel::left(float value)
{
    m_vt = 0;
    m_wt = wmax;
    m_cmdTime = value * M_PI / 180 / wmax;
    m_busy = true;
    if (m_tracePath)
        m_pathTraced << "rot " + QString::number(value);
}

void RobotModel::penUp()
{
    m_tracePath = false;
    setPenEnabled(false);
}

void RobotModel::penDown()
{
    if (!m_tracePath)
    {
        m_tracePath = true;
        m_pathTraced.clear();
        m_pathTraced << QString("pose %1 %2 %3").arg(m_x).arg(m_y).arg(m_phi);
        m_pathTraced << "col " + QString::number(m_penColor.rgba());
    }
    setPenEnabled(true);
}

void RobotModel::arc(float radius, float degrees)
{
    m_vt = degrees > 0? vmax: -vmax;
    m_wt = m_vt / (radius * 0.01f);
    if (m_wt > wmax)
    {
        m_vt *= wmax / m_wt;
        m_wt = wmax;
    }
    else if (m_wt < -wmax)
    {
        m_vt *= -wmax / m_wt;
        m_wt = -wmax;
    }
    m_cmdTime = fabs(degrees * M_PI / 180 / m_wt);
    m_busy = true;
    if (m_tracePath)
        m_pathTraced << QString("arc %1 %2").arg(radius).arg(degrees);
}

void RobotModel::clearScreen()
{
    reset();
    m_cmdTime = 0.25f;
    m_busy = true;
    m_pathTraced.clear();
    emit needClearScreen();
}

void RobotModel::stop()
{
    m_vt = 0;
    m_wt = 0;
    m_cmdTime = 0;
    m_busy = true;
}

void RobotModel::setColor(unsigned int rgb)
{
    m_cmdTime = 0.1f;
    m_busy = true;
    if (rgb == 0xFFFFFFFF)
    {
        m_colorHue = m_penColor.hue();
        m_rainbow = true;
    }
    else
    {
        setPenColor(QColor::fromRgb(rgb));
    }
}

void RobotModel::putchar(char c)
{
    if (m_screenY >= 64/8)
    {
        --m_screenY;
//        for (int y=0; y<64-8; y++)
            memcpy(m_screenImg.scanLine(0), m_screenImg.scanLine(8), 128*(64-8)*4);
        for (int y=64-8; y<64; y++)
        {
            for (int x=0; x<128; x++)
                m_screenImg.setPixel(x, y, 0xFFFFFFFF);
        }
    }

    if (c == '\n')
    {
        m_screenY++;
        m_screenX = 0;
    }
    if ((uint8_t)c >= 0x20)
    {
        int x = m_screenX * 8;
        int y = m_screenY * 8;
        const uint8_t *b = Font8x8_Table + (((uint8_t)c - 0x20) << 3);
        for (int i=0; i<8; i++, y++)
        {
            for (int j=0; j<8; j++)
            {
                if (b[i] & (1<<j))
                {
                    m_screenImg.setPixel(x + 7 - j, y, 0xFF000000);
                }
            }
        }
        m_screenX++;
        if (m_screenX >= 128/8)
        {
            m_screenX = 0;
            m_screenY++;
        }
    }
}

void RobotModel::print(const char *s)
{
    setBalloonColor(Qt::cyan);
    setBalloonVisible(true);
    while (*s)
        putchar(*s++);
    putchar('\n');
    m_needScreenUpdate = true;
}

void RobotModel::cls()
{
    m_screenX = m_screenY = 0;
    m_screenImg.fill(Qt::white);
    setBalloonVisible(false);
    m_needScreenUpdate = true;
}

void RobotModel::showError(const char *message)
{
    QString s = "ОШИБКА:";
    cls();
    print(s.toLocal8Bit().constData());
    print(message);
    setBalloonColor(QColor(255, 32, 0));
}

void RobotModel::sound(float freq, float dur)
{
    m_sound->beep(freq, dur);
    m_cmdTime = dur;
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

    m_x += m_v * cosf(m_phi) * dt/2;
    m_y += m_v * sinf(m_phi) * dt/2;
    m_phi += m_w * dt;
    m_x += m_v * cosf(m_phi) * dt/2;
    m_y += m_v * sinf(m_phi) * dt/2;

    mWheelL->setZRot(-m_phiL * 180 / M_PI);
    mWheelR->setZRot(-m_phiR * 180 / M_PI);

    setPose(m_x, m_y, m_phi);
    updateFace();
    updateScreen();
    updateStar();

    if (m_magic->isVisible())
        updateMagic(dt);

    if (m_rainbow)
    {
        m_colorHue++;
        if (m_colorHue >= 360)
            m_colorHue = 0;
        setPenColor(QColor::fromHsl(m_colorHue, 255, 128));
    }

    if (mBalloon->isVisible())
    {
        float mat[16];
        mBase->findGlobalTransform(mat);
        QMatrix4x4 T(mat);
        QQuaternion q = QQuaternion::fromRotationMatrix(T.toGenericMatrix<3, 3>());
        QQuaternion qc = QQuaternion::fromDirection(scene()->camera()->direction(), scene()->camera()->topDir());
        mBalloon->setRotation(q * qc);
    }
}

void RobotModel::setControl(float v, float w)
{
    m_vt = v*0.05f;
    m_wt = w*0.25f;
}

void RobotModel::setProperty(const char *name, float value)
{
    QString sname = QString(name).toLower();
    if (sname == "joy")
        joy = value;
    else if (sname == "mood")
        mood = value;
    else if (sname == "sleepy")
        sleepy = value;
}

float RobotModel::getProperty(const char *name) const
{
    QString sname = QString(name).toLower();
    if (sname == "joy")
        return joy;
    else if (sname == "mood")
        return mood;
    else if (sname == "sleepy")
        return sleepy;
    return NAN;
}

bool RobotModel::hasProperty(const char *name) const
{
    QString sname = QString(name).toLower();
    if (sname == "joy")
        return true;
    else if (sname == "mood")
        return true;
    else if (sname == "sleepy")
        return true;
    return false;
}

void RobotModel::runCommand(const char *name, const char *arg)
{
    QString cmd = name;
    QString a = arg;
    if (cmd == "showWand")
        setStarVisible(true);
    else if (cmd == "hideWand")
        setStarVisible(false);
    else if (cmd == "doMagic" && m_starVisible)
        startMagic();
    else
        emit commandIssued(cmd, a);
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

void RobotModel::setBalloonColor(QColor color)
{
    color.setAlpha(168);
    mBalloon->setColor(color);
    mBalloonArrow->setColor(color);
}

void RobotModel::reset()
{
    m_v = m_w = 0;
    m_vt = m_wt = 0;
    m_wL = m_wR = 0;
    m_phiL = m_phiR = 0;
    m_x = m_y = m_phi = 0;
    setPenEnabled(false);
    mWheelL->setZRot(0);
    mWheelR->setZRot(0);
    cls();
    setBalloonColor(Qt::cyan);
    m_cmdTime = 0;
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
        if (sleepy < 6)
            eyeY = (rand() % lrintf(6-sleepy)) - 2;
        else
            eyeY = -2;
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
    float ey = (-eyeY * (2*(R-joy))) / 10.0f;

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

    if (joy > 0)
    {
        for (int i=0; i<joy; i++)
        {
            faceImage.setPixel(i/2+1, 9+i, m_eyeColor);
            faceImage.setPixel(i/2+1, 8-i, m_eyeColor);
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

void RobotModel::setPenColor(QColor color)
{
    if (m_penColor != color)
    {
        m_penColor = color;
        m_pathTraced << "col " + QString::number(color.rgb());
        mPen->setColor(color, Qt::white, Qt::black, 0.25, 10);
        qobject_cast<Object3D*>(mPen->children().at(0))->setColor(color);
        qobject_cast<Object3D*>(mPen->children().at(0)->children().at(0))->setColor(color.darker());
    }
}

void RobotModel::setBalloonVisible(bool visible)
{
    if (mBalloonArrow->isVisible() == visible)
        return;
    m_balloonScaleRate = visible? 0.1f: -0.1f;
    m_cmdTime = (1.0f - m_balloonScale) / fabs(m_balloonScaleRate);
    m_busy = true;
    mBalloonArrow->setVisible(visible);
    //    mBalloon->setVisible(visible);
}

void RobotModel::setStarVisible(bool visible)
{
    m_starVisible = visible;
    m_cmdTime = 0.5f;
    m_busy = true;
}

void RobotModel::startMagic()
{
    m_magic->setVisible(true);
    m_magicActive = true;
    m_magicTurns++;
    for (MagicPoint &p: magic)
        p.active = true;
    m_cmdTime = 0.5f;
    m_busy = true;
}

void RobotModel::updateScreen()
{
    if (m_balloonScale < 1.0f && m_balloonScaleRate > 0)
    {
        if (!m_balloonScale)
            mBalloon->setVisible(true);
        m_balloonScale += m_balloonScaleRate;
        if (m_balloonScale >= 1.f)
        {
            m_balloonScale = 1.0f;
            m_busy = false;
        }
        mBalloon->setYScale(m_balloonScale);
    }
    else if (m_balloonScale > 0 && m_balloonScaleRate < 0)
    {
        m_balloonScale += m_balloonScaleRate;
        if (m_balloonScale <= 0)
        {
            m_balloonScale = 0;
            mBalloon->setVisible(false);
            m_busy = false;
        }
        mBalloon->setYScale(m_balloonScale);
    }

    if (m_needScreenUpdate)
    {
        m_needScreenUpdate = false;
        QPainter *p = mScreen->paintBegin();
        p->fillRect(0, 0, 128+12, 64+12, Qt::white);
        p->drawImage(6, 6, m_screenImg);
        mScreen->paintEnd();
    }
}

void RobotModel::updateStar()
{
    float size = m_starSize;
    if (m_starVisible && m_starSize < 1)
        size += 0.05;
    else if (!m_starVisible && m_starSize > 0)
        size -= 0.05;

    if (size != m_starSize)
    {
        m_star->setUniformScale(m_starSize);
        m_starSize = size;
    }
}

void RobotModel::updateMagic(float dt)
{
    GLfloat T[16];
    m_star->findRootTransform(T);
    bool done = true;
    for (MagicPoint &p: magic)
    {
        if (p.active)
        {
            p.x += p.vx * dt;
            p.y += p.vy * dt;
            p.z += p.vz * dt;
            p.vx *= 0.95;
            p.vy *= 0.95;
            p.vz *= 0.95;
            p.vz -= 0.981 * dt;
            done = false;
        }
        if (p.z < 0)
        {
            if (m_magicActive && m_starVisible)
            {
                p.x = T[12] * 0.01;
                p.y = T[13] * 0.01;
                p.z = (T[14] + 2.5) * 0.01;
                p.vx = rnd(0.1);
                p.vy = rnd(0.1);
                p.vz = 0.1 + rnd(0.1);
            }
            else
            {
                p.active = false;
            }
        }
    }

    float spin = m_star->rot().z() + 5;
    if (spin > m_magicTurns * 180)
    {
        m_magicActive = false;
        m_magicTurns = 0;
        m_star->setZRot(0);
    }
    if (m_magicActive)
        m_star->setZRot(spin);

    if (done)
    {
        m_magic->setVisible(false);
    }
}

void RobotModel::updateSheet(QPainter *p)
{
    if (m_penEnabled)
    {
        float pen_width = 0.3f * expf(-m_v*m_v*50); // max 0.3 cm
        p->setPen(QPen(m_penColor, pen_width, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
        if (m_v)
            p->drawLine(QLineF(m_oldx*100, m_oldy*100, m_x*100, m_y*100));
        else
            p->drawPoint(QPointF(m_x*100, m_y*100));

    }
}
