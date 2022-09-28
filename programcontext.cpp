#include "programcontext.h"

ProgramContext::ProgramContext(QString text, ProgramContext *parent) :
    m_parent(parent),
    mPos(0), mOldPos(0),
    m_textOffset(0),
    m_repCount(0),
    m_repMax(0)
{
    QRegExp rx("(;.*\\n|;.*$)");
    rx.setMinimal(true);
    mText = text.replace(rx, "\n").replace('\n', ' ');
    m_textOffset = parent? parent->lastPos()+1: 0;
    if (parent)
    {
        for (QString k: parent->mLocalVars.keys())
            mLocalVars[k] = parent->mLocalVars[k];
    }
    //            restart();
}

bool ProgramContext::isInfixOp(QString token)
{
    QRegExp rx("^([+\\-*/\\^<>=]|>=|<=|<>)$");
    if (token.indexOf(rx) < 0)
        return false;
    return true;
}

QString ProgramContext::testInfixOp()
{
    QRegExp rx("^\\s*([+\\-*/\\^<>=]|>=|<=|<>)");
    int idx = rx.indexIn(mText, mPos, QRegExp::CaretAtOffset);
    if (idx < 0)
        return "";
    return rx.cap(1).toUpper();
}

QString ProgramContext::testNextToken()
{
    QRegExp rx("(\\w+|[\\d.,]+|\\(|\\)|\"\\w+|:\\w+|[+\\-*/\\^<>=]|>=|<=|<>)\\s*");
    int idx = rx.indexIn(mText, mPos);
    if (idx < 0)
        return "";
    return rx.cap(1).toUpper();
}

QString ProgramContext::nextToken()
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

QString ProgramContext::var(QString name) const
{
    if (mLocalVars.contains(name))
        return mLocalVars[name];
    else
        return QString();
}

void ProgramContext::dumpVars() const
{
    qDebug() << "Procedure" << name << "variables:";
    for (QString k: mLocalVars.keys())
    {
        qDebug() << k << "=" << mLocalVars[k];
    }
}

bool ProgramContext::atEnd() const {return mPos >= mText.length();}

ProgramContext *ProgramContext::parent() {return m_parent;}

void ProgramContext::restart()
{
    mPos = 0;
    //            QRegExp sp("^\\s*");
    //            if (mText.indexOf(sp) >= 0)
    //                mPos = sp.matchedLength();
}

bool ProgramContext::iterateLoop()
{
    if (m_repMax && m_repCount < m_repMax)
    {
        m_repCount++;
        restart();
        return true;
    }
    return false;
}

int ProgramContext::lastPos() const {return mOldPos + m_textOffset;}

int ProgramContext::curPos() const {return mPos + m_textOffset - 1;}
