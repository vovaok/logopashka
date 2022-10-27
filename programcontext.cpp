#include "programcontext.h"

QString ProgramContext::regexps[7] =
{
    "^\\s*;.*(\\r?\\n|$)",
    "^\\s*(!=|<>|<=|>=|\\+|\\-|\\*|\\/|\\%|\\^|<|>|=|\\[|\\]|\\(|\\))(\\s*)",
    "^\\s*([a-zA-Zа-яёА-ЯЁ\\.]\\w*\\??)(\\s*)",
    "^\\s*:([a-zA-Zа-яёА-ЯЁ]\\w*)(\\s*)",
    "^\\s*(\\d+(?:\\.\\d+)?)(\\s*)",
    "^\\s*\"([a-zA-Zа-яёА-ЯЁ]\\w*)(\\s*)",
    "^(\\s+)"
};

ProgramContext::ProgramContext(QString text, ProgramContext *parent) :
    m_textOffset(0),
    m_parent(parent),
    m_pos(0), m_oldPos(0),
    m_tokenPos(0), m_tokenEndPos(0)
{
    m_text = text;
    m_textOffset = parent? parent->lastPos()+1: 0;
    if (parent)
    {
        name = parent->name;
        for (QString &k: parent->m_localVars.keys())
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

ProgramContext::Token ProgramContext::testInfixOp()
{
    QRegExp rx(regexps[Token::Ops]);
    int idx = rx.indexIn(m_text, m_pos, QRegExp::CaretAtOffset);
    if (idx < 0)
        return Token::Empty;
    QString op = rx.cap(1);
    if (op == "(" || op == ")" || op == "[" || op == "]")
        return Token::Empty;
    return Token(Token::Ops, op);
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
    for (int i=0; i<7; i++)
    {
        rx.setPattern(regexps[i]);
        rx.setMinimal(!i); // comment is minimal
        int idx = rx.indexIn(m_text, m_pos, QRegExp::CaretAtOffset);
        if (idx >= 0)
        {
            m_oldPos = idx;
            m_pos = idx + rx.matchedLength();
            if (i == Token::Comment || i == Token::Empty)
                return nextToken();
            m_tokenPos = rx.pos(1);
            m_tokenEndPos = rx.pos(2);
            QString body = rx.cap(1);
            if (body == "[")
                return fetchListOrExpr(false);
            else if (body == "(")
                return fetchListOrExpr(true);
            return Token(static_cast<Token::Type>(i), body.toUpper());//.replace("Ё", "Е"));
        }
    }
    if (m_pos == m_text.length())
        return Token::Eof;
    m_oldPos = m_pos;
    rx.setPattern("(\\s|$)");
    int idx = rx.indexIn(m_text, m_pos, QRegExp::CaretAtOffset);
    if (idx >= 0)
        m_pos += rx.matchedLength();
    return Token(Token::Error, "Неопознанный символ " + m_text.mid(m_oldPos, m_pos - m_oldPos)); // error
}

ProgramContext::Token ProgramContext::fetchListOrExpr(bool isExpr)
{
    int pcnt = isExpr? 1: 0;
    int bcnt = isExpr? 0: 1;

    for (; m_pos < m_text.length() && ((bcnt && !isExpr) || (pcnt && isExpr)); m_pos++)
    {
        if (m_text[m_pos] == '[')
            bcnt++;
        else if (m_text[m_pos] == ']')
            --bcnt;

        if (m_text[m_pos] == '(')
            pcnt++;
        else if (m_text[m_pos] == ')')
            --pcnt;
    }

    // eat spaces
    QRegExp rx("^\\s*");
    if (rx.indexIn(m_text, m_pos, QRegExp::CaretAtOffset) >= 0)
        m_pos += rx.matchedLength();

    if (bcnt > 0)
        return Token(Token::Error, "Не хватает скобки ]");
    else if (bcnt < 0)
        return Token(Token::Error, "Лишняя скобка ]");
    else if (pcnt > 0)
        return Token(Token::Error, "Не хватает скобки )");
    else if (pcnt < 0)
        return Token(Token::Error, "Лишняя скобка )");

    m_tokenEndPos = m_pos;

    return Token(isExpr? Token::Expr: Token::List, m_text.mid(m_oldPos, m_pos - m_oldPos));
}

void ProgramContext::setVar(QString name, QString value)
{
    m_localVars[name] = value;
    for (ProgramContext *ctx = m_parent; ctx; ctx = ctx->m_parent)
    {
        if (ctx->m_localVars.contains(name))
            ctx->m_localVars[name] = value;
    }
}

QString ProgramContext::var(QString name) const
{
    if (m_localVars.contains(name))
        return m_localVars[name];
    else if (m_parent)
        return m_parent->var(name);
    return QString();
}

QString ProgramContext::text(int start, int end) const
{
    return m_text.mid(start, end<0? -1: end-start);
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

void ProgramContext::quit()
{
    m_pos = m_text.length();
}

int ProgramContext::lastPos() const
{
    return m_oldPos + m_textOffset;
}

int ProgramContext::curPos() const
{
    return m_pos + m_textOffset;// - 1;
}

int ProgramContext::tokenPos() const
{
    return m_tokenPos + m_textOffset;
}

int ProgramContext::tokenEndPos() const
{
    return m_tokenEndPos + m_textOffset;
}
