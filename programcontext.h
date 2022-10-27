#ifndef PROGRAMCONTEXT_H
#define PROGRAMCONTEXT_H

#include <QString>
#include <QMap>
#include <QDebug>

class ProgramContext
{
public:
    ProgramContext(QString text, ProgramContext *parent=nullptr);

    class Token : public QString
    {
    public:
        enum Type {Comment=0, Ops, Wrd, Var, Num, Sym, Empty, Error, Eof, Expr, List} m_type;
        Token() : m_type(Eof) {}
        Token(Type type, QString text=QString()) : QString(text), m_type(type) {}
        Type type() const {return m_type;}
        bool isOp() const {return m_type == Ops;}
        bool isVar() const {return m_type == Var;}
        bool isNum() const {return m_type == Num;}
        bool isSym() const {return m_type == Sym;}
        bool isError() const {return m_type == Error;}
        bool isEof() const {return m_type == Eof;}
    };

    Token testInfixOp();
    Token nextToken();

    void dumpVars() const;

    bool atEnd() const;

    ProgramContext *parent();

    void restart();
    void quit();

    int lastPos() const;
    int curPos() const;
    int tokenPos() const;
    int tokenEndPos() const;

    void setVar(QString name, QString value);
    QString var(QString name) const;

    int m_textOffset;
    QString name;
    QMap<QString, QString> m_localVars;

    QString text(int start=0, int end=-1) const;

private:
    ProgramContext *m_parent;
    int m_pos, m_oldPos;
    int m_tokenPos, m_tokenEndPos;
//        int mTextOffset;
    QString m_text;

    static QString regexps[7];

    Token fetchListOrExpr(bool isExpr);
};

#endif // PROGRAMCONTEXT_H
