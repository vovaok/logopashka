#ifndef LOGOINTERPRETER_H
#define LOGOINTERPRETER_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QStack>
#include "turtleinterface.h"
#include "logoprocedure.h"
#include "programcontext.h"

class LogoInterpreter : public QThread
{
    Q_OBJECT

public:
    LogoInterpreter();

    void setTurtle(TurtleInterface *turtle);

    bool execute(QString program, bool debug = false);
    QString result();

    void doDebugStep();

    QString programName();
    int curPos() const {return m_context? m_context->curPos(): -1;}
    int lastProcPos() const {return m_context? m_lastProcTokenPos: -1;}

//    typedef enum
//    {
//        NoError = 0,
//        VariableNotFound
//    } ErrorType;

protected:
    void run() override;

signals:
    void error(int start, int end, QString reason);

private:
    TurtleInterface *m_turtle;

    void evalList();

    using Token = ProgramContext::Token;
    Token nextToken();

    QString eval(Token token);

    QMap<QString, LogoProcedure> proc;

    QMap<QString, QString> m_vars;
    void setVar(QString name, QString value);
    QString var(QString name);

    QStack<QString> m_stack;
    ProgramContext *m_context;

    int opPriority(QString op);
    void createProcedures();

//    int m_errorStart, m_errorEnd;
//    QString m_errorString;
    void raiseError(QString reason);

    int m_lastProcTokenPos;

    bool m_debugMode;
    QWaitCondition m_debugStep;
};

#endif // LOGOINTERPRETER_H
