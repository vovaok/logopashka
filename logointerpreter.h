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

    bool execute(QString program, bool debug = false);
    void stop();
    Result result();

    void setDebugMode(bool enabled);
    void doDebugStep();

    QString programName();

    bool isErrorState() const {return m_errorState;}

//    typedef enum
//    {
//        NoError = 0,
//        VariableNotFound
//    } ErrorType;

protected:
    void run() override;

signals:
    void procedureFetched(int start, int end);
    void error(int start, int end, QString reason);

private:
    TurtleInterface *m_turtle;

    void evalList();

    using Token = ProgramContext::Token;
    Token nextToken();

    Result eval(Token token);

    QMap<QString, LogoProcedure> proc;

//    QMap<QString, QString> m_vars;
//    void setVar(QString name, QString value);
//    QString var(QString name);

    QStack<QString> m_stack;
    ProgramContext *m_context;

    int opPriority(QString op);
    void createProcedures();

//    int m_errorStart, m_errorEnd;
//    QString m_errorString;
    void raiseError(QString reason);

    bool m_debugMode;
    QWaitCondition m_debugStep;

    bool m_errorState;
};

#endif // LOGOINTERPRETER_H
