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
    createProcedures();
}

bool LogoInterpreter::execute(QString program)
{
    if (isRunning())
        return false;

    m_context = new ProgramContext(program);
    start();
//    m_stack.clear();
    return true;
}

QString LogoInterpreter::result()
{
    if (m_stack.isEmpty())
        return QString();
    else
        return m_stack.last();
}

void LogoInterpreter::run()
{
    m_stack.clear();

    m_stack.push("23");
}

void LogoInterpreter::setVar(QString name, QString value)
{
    m_vars[name] = value;
}

QString LogoInterpreter::var(QString name)
{
    if (m_vars.contains(name))
        return m_vars[name];
    raiseError(VariableNotFound);
    return QString();
}

void LogoInterpreter::raiseError(Error error)
{
    qWarning() << "LOGO interpreter error" << error;
}

void LogoInterpreter::createProcedures()
{
    proc["ВПЕРЕД"] = [=](QString value)
    {
        m_turtle->forward(value.toFloat());
        while (m_turtle->isBusy());
    };
    proc["НАЗАД"] = [=](QString value)
    {
        m_turtle->backward(value.toFloat());
        while (m_turtle->isBusy());
    };
    proc["ВЛЕВО"] = [=](QString value)
    {
        m_turtle->left(value.toFloat());
        while (m_turtle->isBusy());
    };
    proc["ВПРАВО"] = [=](QString value)
    {
        m_turtle->right(value.toFloat());
        while (m_turtle->isBusy());
    };
    proc["ПЕРОПОДНЯТЬ"] = [=]()
    {
        m_turtle->penUp();
        while (m_turtle->isBusy());
    };
    proc["ПП"] = proc["ПЕРОПОДНЯТЬ"];
    proc["ПЕРООПУСТИТЬ"] = [=]()
    {
        m_turtle->penDown();
        while (m_turtle->isBusy());
    };
    proc["ПО"] = proc["ПЕРООПУСТИТЬ"];

    proc["ОЧИСТИТЬЭКРАН"] = [=]()
    {
        m_turtle->clearScreen();
    };

    proc["ПОВТОР"] = [=](QString count, QString list)
    {
        m_context->dumpVars();
        m_context = new ProgramContext(list, m_context);
        m_context->dumpVars();
        m_context->m_repMax = count.toInt();
        m_context->m_repCount = 1;
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
        QString procName = m_context->nextToken();
        QStringList paramNames;
        while (m_context->testNextToken().startsWith(":"))
            paramNames << m_context->nextToken().mid(1);
        QString text = "";
        int offset = 0;
        while (!m_context->atEnd())
        {
            QString token = m_context->nextToken();
            if (!offset)
                offset = m_context->lastPos();
            if (token == "КОНЕЦ")
                break;
            text += token.toUpper() + " ";
        }
        qDebug() << "name" << procName << "params" << paramNames << "text:";
        qDebug() << text;
        int pcount = paramNames.count();
        proc[procName].setGeneric(pcount, [=](QStringList params)
        {
            QString textcopy = text;
//            for (int i=0; i<pcount; i++)
//                textcopy.replace(paramNames[i], params[i]);
            qDebug() << "STACK" << m_stack;
            ProgramContext *oldContext = m_context;
            ProgramContext *newContext = new ProgramContext(textcopy, m_context);
            newContext->m_textOffset = offset;
            newContext->name = procName;
            for (int i=0; i<pcount; i++)
            {
                newContext->mLocalVars[paramNames[i]] = params[i];
            }
            for (QString &k: newContext->mLocalVars.keys())
            {
                qDebug() << procName << "::" << k << "=" << newContext->mLocalVars[k];
            }
            m_context = newContext;
            if (m_stack.size() > 1) // => wait result => eval here
            {
                QString result;
                do
                {
                    result = eval(m_context->nextToken());
                } while (m_context == newContext && !m_context->atEnd());
                if (m_context != oldContext)
                {
                    delete m_context;
                    m_context = oldContext;
                }
                return result;
            }
            return QString();
        });
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

QString LogoInterpreter::evalExpr(QString expr)
{

}

QString LogoInterpreter::eval(QString token, bool waitOperand, bool dontTestInfix)
{

}
