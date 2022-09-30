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
        enum Type {Comment=0, Ops, Wrd, Var, Num, Sym, Error, Eof, Expr, List} m_type;
        Token() : m_type(Eof) {}
        Token(Type type, QString text=QString()) : m_type(type), QString(text) {}
        Type type() const {return m_type;}
        bool isOp() const {return m_type == Ops;}
        bool isVar() const {return m_type == Var;}
        bool isNum() const {return m_type == Num;}
        bool isSym() const {return m_type == Sym;}
        bool isError() const {return m_type == Error;}
        bool isEof() const {return m_type == Eof;}
    };

//    bool isInfixOp(QString token);
    QString testInfixOp();

//    QString testNextToken();
    Token nextToken();

    QString var(QString name) const;

    void dumpVars() const;

    bool atEnd() const;

    ProgramContext *parent();

    void restart();

    bool iterateLoop();

    int lastPos();
    int curPos();

//        void setVar(QString name, QString value) {mVars[name] = value;}
//        QString var(QString name)
//        {
//            if (mVars.contains(name))
//                return mVars[name];
//            else if (mParent)
//                return mParent->var(name);
//            return "";
//        }

//        QString mLastOp;
//        QStringList mStack;
    int m_textOffset;
    int m_repCount, m_repMax;
    QString name;
    QMap<QString, QString> m_localVars;



private:
    ProgramContext *m_parent;
    int m_pos, m_oldPos;
//        int mTextOffset;
    QString m_text;

    static QString regexps[6];
};

#endif // PROGRAMCONTEXT_H
