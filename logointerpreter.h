#ifndef LOGOINTERPRETER_H
#define LOGOINTERPRETER_H

#include <QThread>
#include <QStack>
#include "turtleinterface.h"
#include "logoprocedure.h"
#include "programcontext.h"

class LogoInterpreter : public QThread
{
public:
    LogoInterpreter();

    bool execute(QString program);

    QString result();

    typedef enum
    {
        NoError = 0,
        VariableNotFound
    } Error;

protected:
    void run() override;

private:
    TurtleInterface *m_turtle;

    using Token = ProgramContext::Token;
    Token nextToken();

    QMap<QString, LogoProcedure> proc;

    QMap<QString, QString> m_vars;
    void setVar(QString name, QString value);
    QString var(QString name);

    QStack<QString> m_stack;
    ProgramContext *m_context;

    int opPriority(QString op);
    void createProcedures();

    QString evalExpr(QString expr);
    void evalList();
    QString eval(Token token);

    void raiseError(Error error);
};

#endif // LOGOINTERPRETER_H
