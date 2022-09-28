#ifndef PROGRAMCONTEXT_H
#define PROGRAMCONTEXT_H

#include <QString>
#include <QMap>
#include <QDebug>

class ProgramContext
{
public:
    ProgramContext(QString text, ProgramContext *parent=nullptr);

    bool isInfixOp(QString token);
    QString testInfixOp();

    QString testNextToken();
    QString nextToken();

    QString var(QString name) const;

    void dumpVars() const;

    bool atEnd() const;

    ProgramContext *parent();

    void restart();

    bool iterateLoop();

    int lastPos() const;
    int curPos() const;

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
    QMap<QString, QString> mLocalVars;

private:
    ProgramContext *m_parent;
    int mPos, mOldPos;
//        int mTextOffset;
    QString mText;
};

#endif // PROGRAMCONTEXT_H
