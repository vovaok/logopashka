#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#if defined(ONB)
#include "objnetvirtualserver.h"
#include "objnetvirtualinterface.h"
#include "objnetmaster.h"
#include "robot.h"
#endif
#include "joystickwidget.h"
#include "led.h"
#include "scene.h"
#ifdef Q_OS_WIN
//#include "spacemouse.h"
#endif
//#include <QRotationSensor>

#include "codeeditor.h"
#include "logointerpreter.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    bool isRunning() const {return logo->isRunning();}

protected slots:
    virtual void closeEvent(QCloseEvent *e) override;
    virtual void keyPressEvent(QKeyEvent *e) override;

private:
    Ui::MainWindow *ui;

    LogoInterpreter *logo;

#if defined(ONB)
    ObjnetVirtualServer *onbvs;
    ObjnetVirtualInterface *onbvi;

    ObjnetMaster *oviMaster;
    Robot *device;
    QPushButton *enableBtn;
#endif

    Scene *scene;
    TurtleInterface *turtle;

    QGroupBox *sceneBox;
    QPushButton *btnScene;
    QPushButton *btnPU, *btnPD;

//    #ifdef Q_OS_WIN
//    SpaceMouse *joy3D;
//    #endif

    CodeEditor *editor;
    QLineEdit *console;

    //QRotationSensor *imu;

    QVBoxLayout *mProgramLayout;
    QGroupBox *mProgramGroup;
    QLabel *mSpacer;

    Led *connLed;
    QLabel *connlabel;

    QStringList mPrograms;
    QString mProgramName;
    QMap<QString, QPushButton*> mProgramBtns;

    JoystickWidget *joy;

    bool mProcessing;
    bool mDebug;


private slots:
    void onTimer();

    void run();

    void step();
    void stop();

    void onLogoProcedureFetched(int start, int end);
    void onLogoError(int start, int end, QString reason);

    void listPrograms();
    void save();
    void open(QString name);
    QString load(QString name);

    void setDebugMode(bool enabled);
};
#endif // MAINWINDOW_H
