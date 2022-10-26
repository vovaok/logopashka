#include "logoprocedure.h"

LogoProcedure::LogoProcedure() :
    mTextOffset(0),
    mIsNative(false),
    mParamCount(0)
{

}

QString LogoProcedure::operator()(QStringList params)
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

void LogoProcedure::setGeneric(int paramCount, LogoProcedure::FuncGeneric f)
{
    mParamCount = paramCount;
    mIsNative = false;
    funcGeneric = f;
}
