#include "scene.h"

Scene::Scene()
{
    //float L[6] = {0.15, 0.3, 0.2}

//    setGeometry(0, 0, 256, 256);
    setMinimumSize(256, 256);
    setBackColor(QColor(240, 240, 240));
    //setViewType(QPanel3D::fly);
    setAutoUpdate(false);
    root()->showAxes(true);

    mMainCam = new Camera3D(this);
    mMainCam->setPosition(QVector3D(-180, 60, 60));
    mMainCam->setTarget(QVector3D(0, 0, 60));
    mMainCam->setTopDir(QVector3D(0, 0, 1));
    mMainCam->setDistanceLimit(500);
    mMainCam->setZoom(1);

    setCamera(mMainCam);

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

    // floor texture:
    QImage img(1000, 1000, QImage::Format_ARGB32_Premultiplied);
    int imgcx = img.width() / 2;
    int imgcy = img.height() / 2;
    QPainter p(&img);
    p.setPen(Qt::NoPen);
    int cellSize = 50;
    int cellStartX = imgcx - (imgcx / cellSize + 1) * cellSize;
    int cellStartY = imgcy - (imgcy / cellSize + 1) * cellSize;
    for (int i=cellStartX; i<img.width(); i+=cellSize)
    {
        for (int j=cellStartY; j<img.height(); j+=cellSize)
        {
            bool c = ((i + j) / cellSize) & 1;
            QColor col = c? QColor(255, 255, 255, 255): QColor(224, 224, 224, 255);
            p.fillRect(i, j, cellSize, cellSize, col);
        }
    }
    p.end();

    mFloor = new Primitive3D(root());
    mFloor->setPlane(QVector3D(100, 0, 0), QVector3D(0, 100, 0));
    mFloor->setDetalization(200, 200);
    mFloor->setColor(QColor(255, 255, 255));
    mFloor->setTexture(new StaticTexture(this, img));

    mBase = new Mesh3D(root());
    mBase->loadModel(":/models/0101.wrl");
    mBase->mesh()->translate(QVector3D(-88, -145.8, 0));
    mBase->mesh()->rotate(90, QVector3D(0, -1, 0));
    mBase->updateModel();

    mLink[0] = new Mesh3D(mBase);
    mLink[0]->loadModel(":/models/0010.wrl");
    mLink[0]->mesh()->translate(QVector3D(1, -11.4, -11.8));
    mLink[0]->mesh()->rotate(90, QVector3D(0, -1, 0));
    mLink[0]->mesh()->rotate(180, QVector3D(0, 0, 1));
    mLink[0]->updateModel();
    mLink[0]->setPosition(0, 0, 10);

    mLink[1] = new Mesh3D(mLink[0]);
    mLink[1]->loadModel(":/models/0020.wrl");
    mLink[1]->mesh()->translate(QVector3D(16.1, -22.3, 7.3));
    mLink[1]->mesh()->rotate(90, QVector3D(1, 0, 0));
    mLink[1]->updateModel();
    mLink[1]->setPosition(0, 0, 16);
    mLink[1]->setOrient(90, 0, 90);

    mLink[2] = new Mesh3D(mLink[1]);
    mLink[2]->loadModel(":/models/0030.wrl");
    mLink[2]->mesh()->translate(QVector3D(-181.75, -69.8, 0));
    mLink[2]->mesh()->rotate(90, QVector3D(-1, 0, 0));
    mLink[2]->updateModel();
    mLink[2]->setPosition(34, 0, 0);
    mLink[2]->setOrient(180, 0, 0);

    mLink[3] = new Mesh3D(mLink[2]);
    mLink[3]->loadModel(":/models/0040.wrl");
    mLink[3]->mesh()->translate(QVector3D(0.3, -9, -8.25));
    mLink[3]->mesh()->rotate(90, QVector3D(0, -1, 0));
    mLink[3]->mesh()->rotate(90, QVector3D(0, 0, 1));
    mLink[3]->updateModel();
    mLink[3]->setPosition(21.5, 0, 0);
    mLink[3]->setOrient(0, 90, 0);

    mLink[4] = new Mesh3D(mLink[3]);
    mLink[4]->loadModel(":/models/0040.wrl");
    mLink[4]->setCenter(0, 0, -11.5);
    mLink[4]->setOrient(180, 0, 180);
    mLink[4]->setPosition(0, 0, 11.5);
    mLink[4]->showAxes(true);

    mLink[5] = new Mesh3D(mLink[4]);
    mLink[5]->loadModel(":/models/0060.wrl");
    mLink[5]->mesh()->translate(QVector3D(-1.25, -4.7, -4.55));
    mLink[5]->mesh()->rotate(0, QVector3D(-1, 0, 0));
    mLink[5]->updateModel();
    mLink[5]->setOrient(0, 90, 0);

    Primitive3D *sph = new Primitive3D(mLink[5]);
    sph->setSphere(2.5);
    sph->setColor(Qt::green);
    sph->setPosition(15, 0, 0);

    mTest = new Primitive3D(root());
    mTest->setSphere(2.5);
    mTest->setColor(Qt::red);
}

void Scene::setAngles(float *q)
{
    mLink[0]->setZRot(-q[0] * 180/M_PI);
    mLink[1]->setYRot(q[1] * 180/M_PI);
    mLink[2]->setZRot(q[2] * 180/M_PI);
    mLink[3]->setXRot(-q[3] * 180/M_PI);
    mLink[4]->setXRot(q[4] * 180/M_PI);
    mLink[5]->setZRot(q[5] * 180/M_PI);
}

void Scene::setTestPos(QVector3D p)
{
    mTest->setPosition(p);
}
