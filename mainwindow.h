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
#include <QGamepad>
#include <QGamepadManager>
//#include <QRotationSensor>

#include "codeeditor.h"
#include "consoleedit.h"
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

    QPushButton *m_btnRun, *m_btnStop, *m_btnSave, *m_btnClose;
    QPushButton *m_btnCamMain, *m_btnCamTop, *m_btnCamFollow, *m_btnCamChase;

//    #ifdef Q_OS_WIN
//    SpaceMouse *joy3D;
//    #endif

    CodeEditor *editor;
    ConsoleEdit *console;

    //QRotationSensor *imu;

//    QVBoxLayout *mProgramLayout;
//    QGroupBox *mProgramGroup;
//    QLabel *mSpacer;

    QListView *mProgramListView;
    QStringListModel *mProgramListModel;

    QListView *mCommandListView;
    QStringListModel *mCommandListModel;

    Led *connLed;
    QLabel *connlabel;

    QStringList mPrograms;
    QString mProgramName;

    JoystickWidget *joy;
    QGamepad *gamepad;

    bool mProcessing;
    bool mDebug;

    QString m_nativeCommandsText;

    QString path(QString name) const;

private slots:
    void onTimer();

    void run();

    void step();
    void stop();

    void onLogoProcedureFetched(int start, int end);
    void onLogoError(QString programName, int start, int end, QString reason);

    void listPrograms();
    void save();
    void open(QString name);
    QString load(QString name);

    void updateCommands();

    void setDebugMode(bool enabled);
    void showScene();
};
#endif // MAINWINDOW_H
