#ifndef LOGOPROCEDURE_H
#define LOGOPROCEDURE_H

#include <QStringList>
#include <functional>

class LogoProcedure
{
private:
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
    LogoProcedure();
    QString programName() const {return mProgramName;}
    int textOffset() const {return mTextOffset;}
    int textLength() const {return mTextLength;}
    bool isNative() const {return mIsNative;}
    bool isFunc() const {return mIsFunc;}
    int paramCount() const {return mParamCount;}
    QStringList aliases() const {return mAliases;}

protected:
    friend class LogoInterpreter;
    void operator =(Proc0 f) {mParamCount = 0; mIsFunc = false; mIsNative = true; proc0 = f;}
    void operator =(Proc1 f) {mParamCount = 1; mIsFunc = false; mIsNative = true; proc1 = f;}
    void operator =(Proc2 f) {mParamCount = 2; mIsFunc = false; mIsNative = true; proc2 = f;}
    void operator =(Proc3 f) {mParamCount = 3; mIsFunc = false; mIsNative = true; proc3 = f;}
    void operator =(Func0 f) {mParamCount = 0; mIsFunc = true; mIsNative = true; func0 = f;}
    void operator =(Func1 f) {mParamCount = 1; mIsFunc = true; mIsNative = true; func1 = f;}
    void operator =(Func2 f) {mParamCount = 2; mIsFunc = true; mIsNative = true; func2 = f;}
    void setGeneric(int paramCount, FuncGeneric f);

    QString operator()(QStringList params);

    QString mProgramName;
    int mTextOffset;
    int mTextLength;
    QStringList mAliases;

private:
    bool mIsFunc;
    bool mIsNative;
    int mParamCount;
};

#endif // LOGOPROCEDURE_H
