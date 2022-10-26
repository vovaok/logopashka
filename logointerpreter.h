#ifndef LOGOINTERPRETER_H
#define LOGOINTERPRETER_H

#include <QThread>
#include <QWaitCondition>
#include <QMutex>
#include <QStack>
#include <QRandomGenerator>
#include "turtleinterface.h"
#include "logoprocedure.h"
#include "programcontext.h"
#include <math.h>

class LogoInterpreter : public QThread
{
    Q_OBJECT

public:
    LogoInterpreter();

    void setTurtle(TurtleInterface *turtle);

    class Result : public QString
    {
    public:
        enum Type {Value, Void, Error};
        Type type() const {return m_type;}
        bool isVoid() const {return m_type == Void;}
        bool isError() const {return m_type == Error;}
        Result() : m_type(Void) {}
        Result(const QString &data) : QString(data), m_type(Value) {}
        Result(Type type, const QString &data=QString()) : QString(data), m_type(type) {}

    private:
        Type m_type;
    };

    bool execute(QString program, QString name, bool debug = false);
    void stop();
    Result result();

    void extractProcedures(QString programText, QString programName);

    void setDebugMode(bool enabled);
    void doDebugStep();

    QString programName();
    QStringList procedures() const;
    const LogoProcedure &procInfo(QString procname) {return proc[procname];}

    bool isErrorState() const {return m_errorState;}

protected:
    void run() override;

signals:
    void procedureFetched(int start, int end);
    void error(QString programName, int start, int end, QString reason);

private:
    TurtleInterface *m_turtle;

    void evalList();
    void evalList(QString list);

    using Token = ProgramContext::Token;
    Token nextToken();

    Result eval(Token token);

    QMap<QString, LogoProcedure> proc;
    QMap<QString, QString> m_aliases;

    QStack<QString> m_stack;
    ProgramContext *m_context;
    int m_lastPrecedence;

    int opPriority(QString op);
    void createProcedures();
    bool createAlias(QString procName, QString alias);
    Result infixOp(QString left, Token op, QString right);
    Result infixOp(int left, Token op, int right);
    Result infixOp(double left, Token op, double right);

    void raiseError(QString reason);

    bool m_debugMode;
    QWaitCondition m_debugStep;

    bool m_errorState;

    QString removeBrackets(QString list);
};

#endif // LOGOINTERPRETER_H
