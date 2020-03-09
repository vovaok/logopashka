#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include "objnetvirtualserver.h"
#include "objnetvirtualinterface.h"
#include "objnetmaster.h"
#include "joystickwidget.h"
#include "robot.h"
#ifdef Q_OS_WIN
//#include "spacemouse.h"
#endif
//#include <QRotationSensor>

#include "codeeditor.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool isRunning() const {return mScriptIndex >= 0;}

protected slots:
    virtual void keyPressEvent(QKeyEvent *e) override;

private:
    Ui::MainWindow *ui;

    ObjnetVirtualServer *onbvs;
    ObjnetVirtualInterface *onbvi;

    ObjnetMaster *oviMaster;
    Robot *device;

//    #ifdef Q_OS_WIN
//    SpaceMouse *joy3D;
//    #endif

    CodeEditor *editor;

    //QRotationSensor *imu;

    QStackedWidget *stack;

    QLabel *connlabel;
    QPushButton *enableBtn;

    JoystickWidget *joy;

//    QList<NamedPose> mScript;
//    QTableWidget *mScriptTable;
    QStringList mScript;
    int mScriptIndex;
    bool mProcessing;
    bool mDebug;

    int mLoopCount;
    int mLoopStartLine;
    int mLoopEndLine;

    void execLine(int num);
    bool parseLine(QString line);

private slots:
    void onTimer();

    void run();
    void step();
    void stop();
};
#endif // MAINWINDOW_H
