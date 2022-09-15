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
#include "scene.h"
#ifdef Q_OS_WIN
//#include "spacemouse.h"
#endif
//#include <QRotationSensor>

#include "codeeditor.h"
#include <functional>

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

    Scene *scene;
    QGroupBox *sceneBox;
    QPushButton *btnScene;
    QPushButton *btnPU, *btnPD;

//    #ifdef Q_OS_WIN
//    SpaceMouse *joy3D;
//    #endif

    CodeEditor *editor;

    //QRotationSensor *imu;

    QStackedWidget *stack;
    QVBoxLayout *mProgramLayout;
    QGroupBox *mProgramGroup;
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
        bool mIsNative;
        int mParamCount;
        //explicit Proc(int paramCount);
        typedef std::function<void()> Proc0;
        typedef std::function<void(QString)> Proc1;
        typedef std::function<void(QString, QString)> Proc2;
        typedef std::function<void(QString, QString, QString)> Proc3;
        typedef std::function<QString()> Func0;
        typedef std::function<QString(QString)> Func1;
        typedef std::function<QString(QString, QString)> Func2;
        typedef std::function<QString(QStringList)> FuncGeneric;
        Proc0 proc0;
        Proc1 proc1;
        Proc2 proc2;
        Proc3 proc3;
        Func0 func0;
        Func1 func1;
        Func2 func2;
        FuncGeneric funcGeneric;
    public:
        Proc(){}
        void operator =(Proc0 f) {mParamCount = 0; mIsFunc = false; mIsNative = true; proc0 = f;}
        void operator =(Proc1 f) {mParamCount = 1; mIsFunc = false; mIsNative = true; proc1 = f;}
        void operator =(Proc2 f) {mParamCount = 2; mIsFunc = false; mIsNative = true; proc2 = f;}
        void operator =(Proc3 f) {mParamCount = 3; mIsFunc = false; mIsNative = true; proc3 = f;}
        void operator =(Func0 f) {mParamCount = 0; mIsFunc = true; mIsNative = true; func0 = f;}
        void operator =(Func1 f) {mParamCount = 1; mIsFunc = true; mIsNative = true; func1 = f;}
        void operator =(Func2 f) {mParamCount = 2; mIsFunc = true; mIsNative = true; func2 = f;}
        void setGeneric(int paramCount, FuncGeneric f) {mParamCount = paramCount; mIsNative = false; funcGeneric = f;}
        int paramCount() const {return mParamCount;}
        QString operator()(QStringList params)
        {
            if (!mIsNative)
                return funcGeneric(params);

            //qDebug() << "params" << params.join(", ");
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
            else if (mParamCount == 3)
                proc3(params[0], params[1], params[2]);
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
        return QString();
    }

    class ScriptContext
    {
    private:
        int mPos, mOldPos;
//        int mTextOffset;
        QString mText;
        ScriptContext *mParent;

    public:
//        QString mLastOp;
//        QStringList mStack;
        int mTextOffset;
        int mRepCount, mRepMax;
        QString name;
        QMap<QString, QString> mLocalVars;

        ScriptContext(QString text, ScriptContext *parent=nullptr) :
            mPos(0), mOldPos(0),
            mTextOffset(0),
            mParent(parent),
            mRepCount(0),
            mRepMax(0)
        {
            QRegExp rx("(;.*\\n|;.*$)");
            rx.setMinimal(true);
            mText = text.replace(rx, "\n").replace('\n', ' ');
            mTextOffset = parent? parent->lastPos()+1: 0;
            if (parent)
            {
                for (QString k: parent->mLocalVars.keys())
                    mLocalVars[k] = parent->mLocalVars[k];
            }
//            restart();
        }

        bool isInfixOp(QString token)
        {
            QRegExp rx("^([+\\-*/\\^<>=]|>=|<=|<>)$");
            if (token.indexOf(rx) < 0)
                return false;
            return true;
        }

        QString testInfixOp()
        {
            QRegExp rx("^\\s*([+\\-*/\\^<>=]|>=|<=|<>)");
            int idx = rx.indexIn(mText, mPos, QRegExp::CaretAtOffset);
            if (idx < 0)
                return "";
            return rx.cap(1).toUpper();
        }

        QString testNextToken()
        {
            QRegExp rx("(\\w+|[\\d.,]+|\\(|\\)|\"\\w+|:\\w+|[+\\-*/\\^<>=]|>=|<=|<>)\\s*");
            int idx = rx.indexIn(mText, mPos);
            if (idx < 0)
                return "";
            return rx.cap(1).toUpper();
        }

        QString nextToken()
        {
            QRegExp rx("(\\w+|[\\d.,]+|\\(|\\)|\"\\w+|:\\w+|[+\\-*/\\^<>=]|>=|<=|<>)\\s*");
            int idx = rx.indexIn(mText, mPos);
            if (idx < 0)
            {
                mPos = mText.length();
                return "";
            }
            mOldPos = idx;
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

        QString var(QString name) const
        {
            if (mLocalVars.contains(name))
                return mLocalVars[name];
            else
                return QString();
        }

        void dumpVars() const
        {
            qDebug() << "Procedure" << name << "variables:";
            for (QString k: mLocalVars.keys())
            {
                qDebug() << k << "=" << mLocalVars[k];
            }
        }

        bool atEnd() const {return mPos >= mText.length();}

        ScriptContext *parent() {return mParent;}

        void restart()
        {
            mPos = 0;
//            QRegExp sp("^\\s*");
//            if (mText.indexOf(sp) >= 0)
//                mPos = sp.matchedLength();
        }

        bool iterateLoop()
        {
            if (mRepMax && mRepCount < mRepMax)
            {
                mRepCount++;
                restart();
                return true;
            }
            return false;
        }

        int lastPos() const {return mOldPos + mTextOffset;}
        int curPos() const {return mPos + mTextOffset - 1;}

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

    QStack<QString> mStack;
    ScriptContext *context;

    int opPriority(QString op);
    void createProcedures();

private slots:
    void onTimer();

    void run();
    QString evalExpr(QString expr);
    QString eval(QString token, bool waitOperand=false, bool dontTestInfix=false);
    void step();
    void stop();

    void listPrograms();
    void save();
    void open(QString name);
    QString load(QString name);
};
#endif // MAINWINDOW_H
