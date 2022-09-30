#include "logointerpreter.h"
static int opPriority(QString op)
{
    if (op.isEmpty())
        return 0;
    if (op == "=" || op == "<" || op == ">" || op == "<>" || op == ">=" || op == "<=")
        return 1;
    if (op == "+" || op == "-")
        return 2;
    if (op == "*" || op == "/")
        return 3;
    if (op == "^")
        return 4;
    if (op == "~") // unary minus
        return 5;
    return 0;
}

LogoInterpreter::LogoInterpreter()
{
    m_turtle = nullptr;
    m_context = nullptr;
    m_lastProcTokenPos = 0;
    createProcedures();
}

void LogoInterpreter::setTurtle(TurtleInterface *turtle)
{
    m_turtle = turtle;
}

bool LogoInterpreter::execute(QString program, bool debug)
{
    if (isRunning())
        return false;

    m_context = new ProgramContext(program);
    m_debugMode = debug;
    start();

    return true;
}

QString LogoInterpreter::result()
{
    if (m_stack.isEmpty())
        return QString();
    else
        return m_stack.last();
}

void LogoInterpreter::doDebugStep()
{
    m_debugStep.wakeAll();
}

QString LogoInterpreter::programName()
{
    if (m_context)
        return m_context->name;
    else
        return QString();
}

void LogoInterpreter::run()
{
    m_lastProcTokenPos = 0;
    m_stack.clear();

    evalList();

    qDebug() << "STACK: " << m_stack;
}

void LogoInterpreter::evalList()
{
    Token token;
    do
    {
        token = nextToken();
        if (token == "]")
            token = Token(Token::Error, "Лишняя скобка ]");
        if (token == ")")
            token = Token(Token::Error, "Лишняя скобка )");
        if (token.isError())
        {
            raiseError(token);
            break;
        }

        QString result = eval(token);
        if (!result.isNull())
            m_stack.push(result);
    }
    while (!token.isEof());
}

LogoInterpreter::Token LogoInterpreter::nextToken()
{
    Token token = m_context->nextToken();
    if (token == "(" || token == "[")
    {
        QString end;
        if (token == "(")
        {
            token = Token(Token::Expr, "(");
            end = ")";
        }
        if (token == "[")
        {
            token = Token(Token::List, "[");
            end = "]";
        }
        Token next;
        do
        {
            next = nextToken();
            if (next.isError())
                return next;

            token += " " + next;

            if (next == end)
                break;
        } while (!next.isEof());
        if (next.isEof())
        {
            return Token(Token::Error, "Где-то не хватает скобки " + end);
        }
    }
    return token;
}

QString LogoInterpreter::eval(Token token)
{
    qDebug() << "Evaluating " << token.type() << token;
    QString result;
    if (token.type() == Token::List)
    {
        m_context = new ProgramContext(token.mid(1, token.length() - 2), m_context);
        evalList();
        ProgramContext *temp = m_context;
        m_context = m_context->parent();
        delete temp;
    }
    else if (token.type() == Token::Expr)
    {
        m_context = new ProgramContext(token.mid(1, token.length() - 2), m_context);
        result = eval(nextToken());
//        if (!nextToken().isEof())
//            raiseError("Ожидается конец выражения");// ERROR: extra code
        ProgramContext *temp = m_context;
        m_context = m_context->parent();
        delete temp;
    }
    else if (token.isNum() || token.isSym())
    {
        result = token;
    }
    else if (token.isVar())
    {
        result = m_context->var(token);
        if (result.isNull())
            raiseError("Переменная :" + token + " не найдена");
    }
    else if (proc.contains(token))
    {
        m_lastProcTokenPos = m_context->lastPos();

        if (m_debugMode)
        {
            QMutex moo;
            moo.lock();
            m_debugStep.wait(&moo);
            moo.unlock();
        }

        QStringList params;
        int pcnt = proc[token].paramCount();
        for (int i=params.count(); i<pcnt; i++)
        {
            params << eval(nextToken());
        }
        qDebug() << ">>" << token << params;
        result = proc[token](params);
        qDebug() << "<<" << result;
    }

    return result;
}

void LogoInterpreter::setVar(QString name, QString value)
{
    m_vars[name] = value;
}

QString LogoInterpreter::var(QString name)
{
    if (m_vars.contains(name))
        return m_vars[name];
//    raiseError(VariableNotFound);
    return QString();
}

void LogoInterpreter::raiseError(QString reason)
{
    emit error(m_context->lastPos(), m_context->curPos(), reason);
}

void LogoInterpreter::createProcedures()
{
    proc["ВПЕРЕД"] = [=](QString value)
    {
        if (m_turtle)
        {
            m_turtle->forward(value.toFloat());
            while (m_turtle->isBusy())
                msleep(10);
        }
    };
    proc["НАЗАД"] = [=](QString value)
    {
        if (m_turtle)
        {
            m_turtle->backward(value.toFloat());
            while (m_turtle->isBusy())
                msleep(10);
        }
    };
    proc["ВЛЕВО"] = [=](QString value)
    {
        if (m_turtle)
        {
            m_turtle->left(value.toFloat());
            while (m_turtle->isBusy())
                msleep(10);
        }
    };
    proc["ВПРАВО"] = [=](QString value)
    {
        if (m_turtle)
        {
            m_turtle->right(value.toFloat());
            while (m_turtle->isBusy())
                msleep(10);
        }
    };
    proc["ПЕРОПОДНЯТЬ"] = [=]()
    {
        if (m_turtle)
        {
            m_turtle->penUp();
            while (m_turtle->isBusy())
                msleep(10);
        }
    };
    proc["ПП"] = proc["ПЕРОПОДНЯТЬ"];
    proc["ПЕРООПУСТИТЬ"] = [=]()
    {
        if (m_turtle)
        {
            m_turtle->penDown();
            while (m_turtle->isBusy())
                msleep(10);
        }
    };
    proc["ПО"] = proc["ПЕРООПУСТИТЬ"];

    proc["ОЧИСТИТЬЭКРАН"] = [=]()
    {
        if (m_turtle)
        {
            m_turtle->clearScreen();
            while (m_turtle->isBusy())
                msleep(10);
        }
    };

    proc["ПОВТОР"] = [=](QString count, QString list)
    {
        for (int i=0; i<count; i++)
        {
            qDebug() << list;
//            m_context = new ProgramContext(list.mid(1, list.length() - 2), m_context);
//            evalList();
//            ProgramContext *temp = m_context;
//            m_context = m_context->parent();
//            delete temp;
        }

//        m_context->dumpVars();
//        m_context = new ProgramContext(list, m_context);
//        m_context->dumpVars();
//        m_context->m_repMax = count.toInt();
//        m_context->m_repCount = 1;
    };

    proc["ЕСЛИ"] = [=](QString cond, QString list)
    {
        if (cond.toInt())
            m_context = new ProgramContext(list, m_context);
    };

    proc["ЕСЛИИНАЧЕ"] = [=](QString cond, QString list1, QString list2)
    {
        if (cond.toInt())
            m_context = new ProgramContext(list1, m_context);
        else
            m_context = new ProgramContext(list2, m_context);
    };

    proc["СТОП"] = [=]()
    {
        do
        {
            ProgramContext *oldContext = m_context;
            m_context = m_context->parent();
            delete oldContext;
        } while (m_context && m_context->name.isEmpty());
        if (m_context)
        {
            ProgramContext *oldContext = m_context;
            m_context = m_context->parent();
            delete oldContext;
        }
    };

    proc["ВЫХОД"] = static_cast<std::function<QString(QString)>>([=](QString result)
    {
        ProgramContext *oldContext = m_context;
        m_context = m_context->parent();
        delete oldContext;
        return result;
    });

    proc["ИСПОЛНИТЬ"] = [=](QString name, QString value)
    {
        setVar(name, value);
    };

    proc["ПРОИЗВОЛЬНО"] = static_cast<std::function<QString(QString)>>([=](QString max)
    {
        int maxval = max.toInt();
        int value = maxval? rand() % maxval: 0;
        return QString::number(value);
    });

    proc["СУММА"] = static_cast<std::function<QString(QString, QString)>>([=](QString a, QString b)
    {
        return QString::number(a.toDouble() + b.toDouble());
    });

    proc["РАЗНОСТЬ"] = static_cast<std::function<QString(QString, QString)>>([=](QString a, QString b)
    {
        return QString::number(a.toDouble() - b.toDouble());
    });

    proc["МИНУС"] = static_cast<std::function<QString(QString)>>([=](QString x)
    {
        return QString::number(-x.toDouble());
    });

    proc["ЭТО"] = [=]()
    {
//        QString procName = m_context->nextToken();
//        QStringList paramNames;
//        while (m_context->testNextToken().startsWith(":"))
//            paramNames << m_context->nextToken().mid(1);
//        QString text = "";
//        int offset = 0;
//        while (!m_context->atEnd())
//        {
//            QString token = m_context->nextToken();
//            if (!offset)
//                offset = m_context->lastPos();
//            if (token == "КОНЕЦ")
//                break;
//            text += token.toUpper() + " ";
//        }
//        qDebug() << "name" << procName << "params" << paramNames << "text:";
//        qDebug() << text;
//        int pcount = paramNames.count();
//        proc[procName].setGeneric(pcount, [=](QStringList params)
//        {
//            QString textcopy = text;
////            for (int i=0; i<pcount; i++)
////                textcopy.replace(paramNames[i], params[i]);
//            qDebug() << "STACK" << m_stack;
//            ProgramContext *oldContext = m_context;
//            ProgramContext *newContext = new ProgramContext(textcopy, m_context);
//            newContext->m_textOffset = offset;
//            newContext->name = procName;
//            for (int i=0; i<pcount; i++)
//            {
//                newContext->m_localVars[paramNames[i]] = params[i];
//            }
//            for (QString &k: newContext->m_localVars.keys())
//            {
//                qDebug() << procName << "::" << k << "=" << newContext->m_localVars[k];
//            }
//            m_context = newContext;
//            if (m_stack.size() > 1) // => wait result => eval here
//            {
//                QString result;
//                do
//                {
//                    result = eval(m_context->nextToken());
//                } while (m_context == newContext && !m_context->atEnd());
//                if (m_context != oldContext)
//                {
//                    delete m_context;
//                    m_context = oldContext;
//                }
//                return result;
//            }
//            return QString();
//        });
    };

    proc["ЗАГРУЗИТЬ"] = [=](QString name)
    {
        qDebug() << "load program" << name << "...";
        QString text;
#warning TODO: load is unimplemented!!
//        text = load(name).trimmed();
        ProgramContext *oldContext = m_context;
        m_context = new ProgramContext(text);
        do
        {
            eval(m_context->nextToken());
        } while (!m_context->atEnd());
        delete m_context;
        m_context = oldContext;
        qDebug() << "done";
    };
}


