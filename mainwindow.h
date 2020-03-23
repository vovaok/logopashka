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
        bool mIsFunc;
        int mParamCount;
        //explicit Proc(int paramCount);
        typedef std::function<void()> Proc0;
        typedef std::function<void(QString)> Proc1;
        typedef std::function<void(QString, QString)> Proc2;
        typedef std::function<QString()> Func0;
        typedef std::function<QString(QString)> Func1;
        typedef std::function<QString(QString, QString)> Func2;
        Proc0 proc0;
        Proc1 proc1;
        Proc2 proc2;
        Func0 func0;
        Func1 func1;
        Func2 func2;
    public:
        Proc(){}
        void operator =(Proc0 f) {mParamCount = 0; mIsFunc = false; proc0 = f;}
        void operator =(Proc1 f) {mParamCount = 1; mIsFunc = false; proc1 = f;}
        void operator =(Proc2 f) {mParamCount = 2; mIsFunc = false; proc2 = f;}
        void operator =(Func0 f) {mParamCount = 0; mIsFunc = true; func0 = f;}
        void operator =(Func1 f) {mParamCount = 1; mIsFunc = true; func1 = f;}
        void operator =(Func2 f) {mParamCount = 2; mIsFunc = true; func2 = f;}
        int paramCount() const {return mParamCount;}
        QString operator()(QStringList params)
        {
            qDebug() << "params" << params.join(", ");
            if (mIsFunc)
            {
                if (mParamCount == 0)
                    return func0();
                else if (mParamCount == 1)
                    return func1(params[0]);
                else if (mParamCount == 2)
                    return func2(params[0], params[1]);
            }

            if (mParamCount == 0)
                proc0();
            else if (mParamCount == 1)
                proc1(params[0]);
            else if (mParamCount == 2)
                proc2(params[0], params[1]);
            return "";
        }
    };

    QMap<QString, Proc> proc;

    QMap<QString, QString> mVars;
    void setVar(QString name, QString value) {mVars[name] = value;}
    QString var(QString name)
    {
        if (mVars.contains(name))
            return mVars[name];
        return "";
    }

    class ScriptContext
    {
    private:
        int mPos;
        QString mText;
//        QMap<QString, QString> mVars;
        ScriptContext *mParent;

    public:
        QString mCurToken;
        QStringList mCurParams;
        QString mResult;

        ScriptContext(QString text, ScriptContext *parent=nullptr) :
            mPos(0),
            mParent(parent)
        {
            QRegExp rx("(;.*\\n|;.*$)");
            rx.setMinimal(true);
            mText = text.replace(rx, "\n").replace('\n', ' ');
        }

        QString testInfixOp()
        {
            QRegExp rx("^\\s*([+\\-*/])");
            int idx = rx.indexIn(mText, mPos, QRegExp::CaretAtOffset);
            if (idx < 0)
                return "";
            return rx.cap(1).toUpper();
        }

        QString nextToken()
        {
            QRegExp rx("(\\w+|[\\d.,]+|\\(|\\)|\"\\w+|:\\w+|[+\\-*/])\\s*");
            int idx = rx.indexIn(mText, mPos);
            if (idx < 0)
                return "";
            mPos = idx + rx.matchedLength();
            QString word = rx.cap(1).toUpper();
            if (word == "(")
            {
                int pcnt = 1;
                for (; mPos<mText.length() && pcnt; mPos++)
                {
                    if (mText[mPos] == '(')
                        pcnt++;
                    else if (mText[mPos] == ')')
                        --pcnt;
                }
                word = mText.mid(idx, mPos - idx);
            }
            return word;
        }

        ScriptContext *parent() {return mParent;}

//        void setVar(QString name, QString value) {mVars[name] = value;}
//        QString var(QString name)
//        {
//            if (mVars.contains(name))
//                return mVars[name];
//            else if (mParent)
//                return mParent->var(name);
//            return "";
//        }
    };

    ScriptContext *context;


    void createProcedures();

private slots:
    void onTimer();

    void run();
    QString eval(QString token);
    void step();
    void stop();

    void listPrograms();
    void save();
    void load(QString name);
};
#endif // MAINWINDOW_H
