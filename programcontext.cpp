#include "programcontext.h"

QString ProgramContext::regexps[6] =
{
    "^\\s*;.*(\\r?\\n|$)",
    "^\\s*(!=|<>|<=|>=|\\+|\\-|\\*|\\/|\\%\\^|<|>|=|\\[|\\]|\\(|\\))\\s*",
    "^\\s*([a-zA-Zа-яёА-ЯЁ\\.]\\w*\\??)\\s*",
    "^\\s*:([a-zA-Zа-яёА-ЯЁ]\\w*)\\s*",
    "^\\s*(\\d+(?:\\.\\d+)?)\\s*",
    "^\\s*\"([a-zA-Zа-яёА-ЯЁ]\\w*)\\s*"
};

ProgramContext::ProgramContext(QString text, ProgramContext *parent) :
    m_parent(parent),
    m_pos(0), m_oldPos(0),
    m_textOffset(0),
    m_repCount(0),
    m_repMax(0)
{
    m_text = text;
    m_textOffset = parent? parent->lastPos()+1: 0;
    if (parent)
    {
        for (QString k: parent->m_localVars.keys())
            m_localVars[k] = parent->m_localVars[k];
    }
}

//bool ProgramContext::isInfixOp(QString token)
//{
//    QRegExp rx("^([+\\-*/\\^<>=]|>=|<=|<>)$");
//    if (token.indexOf(rx) < 0)
//        return false;
//    return true;
//}

QString ProgramContext::testInfixOp()
{
    QRegExp rx("^\\s*([+\\-*/\\^<>=]|>=|<=|<>)");
    int idx = rx.indexIn(m_text, m_pos, QRegExp::CaretAtOffset);
    if (idx < 0)
        return "";
    return rx.cap(1).toUpper();
}

//QString ProgramContext::testNextToken()
//{
//    QRegExp rx("(\\w+|[\\d.,]+|\\(|\\)|\"\\w+|:\\w+|[+\\-*/\\^<>=]|>=|<=|<>)\\s*");
//    int idx = rx.indexIn(m_text, m_pos);
//    if (idx < 0)
//        return "";
//    return rx.cap(1).toUpper();
//}

ProgramContext::Token ProgramContext::nextToken()
{
    QRegExp rx;
    for (int i=0; i<6; i++)
    {
        rx.setPattern(regexps[i]);
        rx.setMinimal(!i); // comment is minimal
        int idx = rx.indexIn(m_text, m_pos, QRegExp::CaretAtOffset);
        if (idx >= 0)
        {
            m_oldPos = idx;
            m_pos = idx + rx.matchedLength();
            if (i) // if it is not comment
                return Token(static_cast<Token::Type>(i), rx.cap(1).toUpper());
            else
                return nextToken();
        }
    }
    if (m_pos == m_text.length())
        return Token(Token::Eof, "$"); // end of file
    m_oldPos = m_pos;
    rx.setPattern("(\\s|$)");
    m_pos = rx.indexIn(m_text, m_pos, QRegExp::CaretAtOffset);
    return Token(Token::Error, "Неопознанный символ"); // error
}

QString ProgramContext::var(QString name) const
{
    if (m_localVars.contains(name))
        return m_localVars[name];
    else
        return QString();
}

void ProgramContext::dumpVars() const
{
    qDebug() << "Procedure" << name << "variables:";
    for (QString k: m_localVars.keys())
    {
        qDebug() << k << "=" << m_localVars[k];
    }
}

bool ProgramContext::atEnd() const {return m_pos >= m_text.length();}

ProgramContext *ProgramContext::parent() {return m_parent;}

void ProgramContext::restart()
{
    m_pos = 0;
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

int ProgramContext::lastPos()
{
    int pos = m_oldPos + m_textOffset;
//    for (ProgramContext *ctx = this; ctx; ctx = ctx->parent())
//        pos += ctx->m_textOffset;
    return pos;
}

int ProgramContext::curPos()
{
    int pos = m_pos + m_textOffset;// - 1;
//    for (ProgramContext *ctx = this; ctx; ctx = ctx->parent())
//        pos += ctx->m_textOffset;
    return pos;
}
