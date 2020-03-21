#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtWidgets>
#include "objnetvirtualserver.h"
#include "objnetvirtualinterface.h"
#include "objnetmaster.h"
#include "joystickwidget.h"
#include "led.h"
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

    bool isRunning() const {return context;}

protected slots:
    virtual void closeEvent(QCloseEvent *e) override;
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
    QVBoxLayout *mProgramLayout;
    QLabel *mSpacer;

    Led *connLed;
    QLabel *connlabel;
    QPushButton *enableBtn;

    QStringList mPrograms;
    QString mProgramName;
    QMap<QString, QPushButton*> mProgramBtns;

    QStringList mCallStack;

    JoystickWidget *joy;

    QStringList mScript;
//    int mScriptIndex;
    bool mProcessing;
    bool mDebug;

    class Proc
    {
    private:
        int mParamCount;
        //explicit Proc(int paramCount);
        typedef std::function<void()> Func0;
        typedef std::function<void(QString)> Func1;
        typedef std::function<void(QString, QString)> Func2;
        Func0 exec0;
        Func1 exec1;
        Func2 exec2;
    public:
        Proc(){}
        operator =(Func0 f) {mParamCount = 0; exec0 = f;}
        operator =(Func1 f) {mParamCount = 1; exec1 = f;}
        operator =(Func2 f) {mParamCount = 2; exec2 = f;}
        int paramCount() const {return mParamCount;}
        QString operator()(QStringList params)
        {
            qDebug() << "params" << params.join(", ");
            if (mParamCount == 0)
                exec0();
            else if (mParamCount == 1)
                exec1(params[0]);
            else if (mParamCount == 2)
                exec2(params[0], params[1]);
            return "";
        }
    };

    QMap<QString, Proc> proc;

    struct Loop
    {
        int counter;
        int line;
    };

    class ScriptContext
    {
    private:
        int mPos;
        QString mText;
        QMap<QString, float> mVars;
        ScriptContext *mParent;

    public:
        ScriptContext(QString text, ScriptContext *parent=nullptr) :
            mPos(0),
            mParent(parent)
        {
            mText = text.replace('\n', ' ');
        }

        QString fetchNext()
        {
            QRegExp rx("(\\w+|[\\d.,]+|\\(|\\)|\"\\w+|:\\w+|[+-*/])\\s*");
            int idx = rx.indexIn(mText, mPos);
            if (idx < 0)
                return "";
            mPos = idx + rx.matchedLength();
            QString word = rx.cap(1).toUpper();
            if (word == "(")
            {
                int pcnt = 1;
                int oldpos = mPos;
                for (; mPos<mText.length() && pcnt; mPos++)
                {
                    if (mText[mPos] == '(')
                        pcnt++;
                    else if (mText[mPos] == ')')
                        --pcnt;
                }
                word = mText.mid(oldpos, mPos - oldpos - 1);
            }
            return word;
        }

        ScriptContext *parent() {return mParent;}
    };

    ScriptContext *context;

//    int mLoopCount;
//    int mLoopStartLine;
//    int mLoopEndLine;

//    void execLine();
    bool parseLine(QString line);

    void createProcedures();

private slots:
    void onTimer();

    void run();
    void step();
    void stop();

    void listPrograms();
    void save();
    void load(QString name);

    void exec(QString command);
};
#endif // MAINWINDOW_H
