#include "logointerpreter.h"

static int opPrecedence(QString op)
{
    if (op.isEmpty())
        return 0;
    if (op == "=" || op == "<>" || op == "!=")
        return 1;
    if (op == "<" || op == ">" || op == ">=" || op == "<=")
        return 2;
    if (op == "+" || op == "-")
        return 3;
    if (op == "*" || op == "/" || op == "%")
        return 4;
    if (op == "^")
        return 5;
    if (op == "~") // unary minus
        return 6;
    return 0;
}

LogoInterpreter::LogoInterpreter()
{
    m_turtle = nullptr;
    m_context = nullptr;
    m_errorState = false;
    m_lastPrecedence = 0;
    createProcedures();
}

void LogoInterpreter::setTurtle(TurtleInterface *turtle)
{
    m_turtle = turtle;
}

bool LogoInterpreter::execute(QString program, bool debug)
{
    if (isRunning())
    {
        raiseError("Программа уже запущена!");
        return false;
    }

    if (m_turtle)
        m_turtle->cls();
    m_context = new ProgramContext(program);
    m_errorState = false;
    m_debugMode = debug;
    m_lastPrecedence = 0;

    start();

    return true;
}

void LogoInterpreter::stop()
{
    requestInterruption();
    if (m_debugMode)
        m_debugStep.wakeAll();
    m_debugMode = false;
}

LogoInterpreter::Result LogoInterpreter::result()
{
    if (m_stack.isEmpty())
        return Result();
    else
        return m_stack.last();
}

void LogoInterpreter::setDebugMode(bool enabled)
{
    m_debugMode = enabled;
    if (!m_debugMode)
        m_debugStep.wakeAll();
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
    m_stack.clear();
    evalList();

    m_debugMode = false;
    while (m_context)
    {
        ProgramContext *tmp = m_context;
        m_context = m_context->parent();
        delete tmp;
    }

    qDebug() << "STACK: " << m_stack;
}

void LogoInterpreter::evalList()
{
    Token token;
    do
    {
        token = nextToken();
        if (token == Token::Eof)
            break;
        if (token == "]")
            token = Token(Token::Error, "Лишняя скобка ]");
        if (token == ")")
            token = Token(Token::Error, "Лишняя скобка )");
        if (token.isError())
        {
            raiseError(token);
            break;
        }

        Result result = eval(token);
        if (!result.isVoid() && !result.isError())
            m_stack.push(result);
    }
    while (!m_context->atEnd() && !m_errorState && !isInterruptionRequested());
}

void LogoInterpreter::evalList(QString list)
{
    m_context = new ProgramContext(list, m_context);
    evalList();
    ProgramContext *temp = m_context;
    m_context = m_context->parent();
    delete temp;
}

LogoInterpreter::Token LogoInterpreter::nextToken()
{
    if (isInterruptionRequested())
        return Token::Eof;
    Token token = m_context->nextToken();
    return token;
}

LogoInterpreter::Result LogoInterpreter::eval(Token token)
{
    Result result;
//    qDebug() << "Eval: " /*<< token.type()*/ << token;
//    return result;

    if (token.isError())
    {
        raiseError(token);
        return Result::Error;
    }

    if (token.isEof())
        return result;

    if (token.isOp())
    {
        if (token == "-") // unary -
        {
            token = Token(Token::Wrd, "МИНУС");
        }
        else
        {
            raiseError("Лишний оператор " + token);
            result = Result::Error;
            return result;
        }
    }
    else
    {
        result = token;
    }

    if (token.type() == Token::List || token.isSym())
    {
        return result;
    }

    if (token.isVar())
    {
        result = m_context->var(token);
        if (result.isNull())
            raiseError("Переменная :" + token + " не найдена");
    }
    else if (token.type() == Token::Expr)
    {
        int last = m_lastPrecedence;
        m_lastPrecedence = 0;
        QRegExp rx("^\\s*\\((.*)\\)\\s*$");
        QString expr = token;
        if (rx.indexIn(token) >= 0)
            expr = rx.cap(1);
        m_context = new ProgramContext(expr, m_context);
        result = eval(nextToken());
        if (!nextToken().isEof())
            raiseError("Ожидается конец выражения");// ERROR: extra code
        ProgramContext *temp = m_context;
        m_context = m_context->parent();
        delete temp;
        m_lastPrecedence = last;
    }
    else if (proc.contains(token))
    {
        float lastProcTokenPos = m_context->tokenPos();

        QStringList params;
        int pcnt = proc[token].paramCount();
        for (int i=params.count(); i<pcnt; i++)
        {
            int last = m_lastPrecedence;
            m_lastPrecedence = 100;

            Token tk = nextToken();
            if (tk.isEmpty() || tk.isError() || tk.isEof())
            {
                raiseError("Параметр " + QString::number(i+1) + (tk.isError()? " задан неверно": " не задан"));
                return Result::Error;
            }
            params << eval(tk);

            m_lastPrecedence = last;
        }

        if (m_errorState)
            return Result::Error;

        emit procedureFetched(lastProcTokenPos, m_context->tokenEndPos());

        if (m_debugMode)
        {
            QMutex moo;
            moo.lock();
            m_debugStep.wait(&moo);
            moo.unlock();

            if (isInterruptionRequested())
                return Result::Void;
        }

        qDebug() << ">>" << token << params;
        result = proc[token](params);
        qDebug() << "<<" << result;
    }
    else if (token.type() == Token::Wrd)
    {
        raiseError("Я не умею " + token);
        result = Result::Error;
    }

    Token next = m_context->testInfixOp();
    while (next.isOp())
    {
        if (result.isEmpty() || result.isError())
        {
            raiseError("Нет левой части выражения");
            return Result::Error;
        }

        int prec = opPrecedence(next);
        int last = m_lastPrecedence;
        m_lastPrecedence = prec;
//            qDebug() << "precedence" << prec << last;
        if (prec > last)
        {
            Token op = nextToken();
            Result right = eval(nextToken());
            if (!right.isEmpty() && !right.isError())
                result = infixOp(result, op, right);
            else
            {
                raiseError("Не удалось разобрать выражение");
                return Result::Error;
            }
        }
        else
        {
            m_lastPrecedence = last;
            break;
        }
        m_lastPrecedence = last;
        next = m_context->testInfixOp();
    }

    return result;
}

void LogoInterpreter::raiseError(QString reason)
{
    m_errorState = true;
    emit error(m_context->lastPos(), m_context->curPos(), reason);

    if (m_turtle)
    {
        m_turtle->showError(reason.toLocal8Bit().constData());
    }
}

QString LogoInterpreter::removeBrackets(QString list)
{
    QRegExp rx("^\\s*\\[(.*)\\]\\s*$");
    if (rx.indexIn(list) >= 0)
        return rx.cap(1);
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
    proc["ВПЕРЁД"] = proc["ВПЕРЕД"];
    proc["НАЗАД"] = [=](QString value)
    {
        if (m_turtle)
        {
            m_turtle->backward(value.toFloat());
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
    proc["ВЛЕВО"] = [=](QString value)
    {
        if (m_turtle)
        {
            m_turtle->left(value.toFloat());
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

    proc["ВЫБРАТЬЦВЕТ"] = [=](QString color)
    {
        if (m_turtle)
        {
            uint col = color.toUInt();
            if (color.startsWith('[')) // if this is list
            {
                int oldStackSize = m_stack.size();
                m_context = new ProgramContext(removeBrackets(color), m_context);
                evalList();
                ProgramContext *temp = m_context;
                m_context = m_context->parent();
                delete temp;
                qDebug() << "COLOR STACK: " << m_stack;
                if (m_stack.size() - oldStackSize != 3)
                    raiseError("Список должен содержать ровно 3 элемента");
                else
                {
                    int b = m_stack.pop().toUInt();
                    int g = m_stack.pop().toUInt();
                    int r = m_stack.pop().toUInt();
                    col = b + g * 0x100 + r * 0x10000;
                }
            }
            else
            {
                QMap<QString, uint> colors;
                colors["ЧЕРНЫЙ"] = 0x000000;
                colors["КРАСНЫЙ"] = 0xFF0000;
                colors["ОРАНЖЕВЫЙ"] = 0xFF8000;
                colors["ЖЕЛТЫЙ"] = 0xFFFF00;
                colors["ЖЁЛТЫЙ"] = colors["ЖЕЛТЫЙ"];
                colors["ЗЕЛЕНЫЙ"] = 0x00FF00;
                colors["ЗЕЛЁНЫЙ"] = colors["ЗЕЛЕНЫЙ"];
                colors["ГОЛУБОЙ"] = 0x00FFFF;
                colors["СИНИЙ"] = 0x0040FF;
                colors["ФИОЛЕТОВЫЙ"] = 0x8000FF;
                colors["СИРЕНЕВЫЙ"] = 0xFF00FF;
                colors["БЕЛЫЙ"] = 0xFFFFFF;
                colors["СЕРЫЙ"] = 0x808080;
                colors["РОЗОВЫЙ"] = 0xFFA0A0;
                colors["КОРИЧНЕВЫЙ"] = 0x804000;
                colors["БЕЖЕВЫЙ"] = 0xFFDCBA;
                if (colors.contains(color))
                    col = colors[color];
                else
                    raiseError("Я не знаю такой цвет: " + color);
            }

            m_turtle->setColor(col);
            while (m_turtle->isBusy())
                msleep(10);
        }
    };

    proc["ДУГА"] = [=](QString radius, QString degrees)
    {
        if (m_turtle)
        {
            m_turtle->arc(radius.toDouble(), degrees.toDouble());
            while (m_turtle->isBusy())
                msleep(10);
        }
    };

    proc["ОЧИСТИТЬЭКРАН"] = [=]()
    {
        if (m_turtle)
        {
            m_turtle->clearScreen();
            while (m_turtle->isBusy())
                msleep(10);
        }
    };

    proc["ПЕЧАТЬ"] = [=](QString text)
    {
        if (text.startsWith("["))
            text = removeBrackets(text);
        if (m_turtle)
        {
            m_turtle->print(text.toLocal8Bit().constData());
            while (m_turtle->isBusy())
                msleep(10);
        }
    };

    proc["УБРАТЬЭКРАН"] = [=]()
    {
        if (m_turtle)
        {
            m_turtle->cls();
            while (m_turtle->isBusy())
                msleep(10);
        }
    };

    proc["ЗВУК"] = [=](QString sFreq, QString sDur)
    {
        if (m_turtle)
        {
            bool ok;
            float f = sFreq.toDouble(&ok);
            if (ok && f >= 0 && f <= 20000)
            {
                float d = sDur.toDouble(&ok);
                if (ok && d >= 0 && d <= 10)
                {
                    m_turtle->sound(f, d);
                    while (m_turtle->isBusy())
                        msleep(2);
                    return;
                }
            }
            raiseError("Параметры заданы неверно");
        }
    };

    proc["ПОВТОР"] = [=](QString count, QString list)
    {
        int cnt = count.toInt();
//        m_context->dumpVars();
        m_context = new ProgramContext(removeBrackets(list), m_context);
//        m_context->dumpVars();
        for (int i=0; i<cnt; i++)
        {
            evalList();
            m_context->restart();
        }
        ProgramContext *temp = m_context;
        m_context = m_context->parent();
        delete temp;
    };

    proc["ПОКА"] = [=](QString cond, QString list)
    {
        m_context = new ProgramContext(removeBrackets(list), m_context);
        while (cond.toInt())
        {
            evalList();
            m_context->restart();
        }
        ProgramContext *temp = m_context;
        m_context = m_context->parent();
        delete temp;
    };

    proc["ЕСЛИ"] = [=](QString cond, QString list)
    {
        if (cond.toInt())
            evalList(removeBrackets(list));
    };

    proc["ЕСЛИИНАЧЕ"] = [=](QString cond, QString list1, QString list2)
    {
        if (cond.toInt())
            evalList(removeBrackets(list1));
        else
            evalList(removeBrackets(list2));
    };

    proc["ЖДАТЬ"] = [=](QString sTime)
    {
        bool ok;
        double time_ms = sTime.toDouble(&ok) * 1000;
        if (ok)
            msleep(time_ms);
        else
            raiseError("Параметр задан неверно");
    };

    proc["СТОП"] = [=]()
    {
        stop();
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
        m_context->setVar(name, value);
    };

    proc["ПРОИЗВОЛЬНО"] = static_cast<std::function<QString(QString)>>([=](QString max)
    {
        int maxval = max.toInt();
        uint32_t rnd = QRandomGenerator::global()->generate();
        int value = maxval>0? rnd % maxval: 0;
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
    proc["-"] = proc["МИНУС"];

    proc["ЭТО"] = [=]()
    {
        QString procName = m_context->nextToken();
        QStringList paramNames;
        Token token = m_context->nextToken();
        while (token.isVar())
        {
            paramNames << token;
            token = m_context->nextToken();
        }

        int offset = m_context->lastPos();
        while (token != "КОНЕЦ")
        {
            if (m_context->atEnd())
            {
                raiseError("КОНЕЦ процедуры " + procName + " не найден");
                return;
            }
            token = m_context->nextToken();
        }
        int end = m_context->lastPos();

        QString text = m_context->text(offset, end);


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

        qDebug() << "name" << procName << "params" << paramNames << "text:";
        qDebug() << text;
        int pcount = paramNames.count();
        proc[procName].setGeneric(pcount, [=](QStringList params)
        {
            Result result;
//            qDebug() << "STACK" << m_stack;
            ProgramContext *oldContext = m_context;
            ProgramContext *newContext = new ProgramContext(text, m_context);
            newContext->m_textOffset = offset;
            newContext->name = procName;
            for (int i=0; i<pcount; i++)
            {
                newContext->m_localVars[paramNames[i]] = params[i];
            }
//            for (QString &k: newContext->m_localVars.keys())
//            {
//                qDebug() << procName << "::" << k << "=" << newContext->m_localVars[k];
//            }
            m_context = newContext;

            evalList();

            if (m_context != oldContext)
            {
                delete m_context;
                m_context = oldContext;
            }

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
            return result;
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

    proc["ДЛИНАСПИСКА"] = static_cast<std::function<QString(QString)>>([=](QString list)
    {
        int oldStackSize = m_stack.size();
        m_context = new ProgramContext(removeBrackets(list), m_context);
        evalList();
        ProgramContext *temp = m_context;
        m_context = m_context->parent();
        delete temp;
        int list_size = m_stack.size() - oldStackSize;
        m_stack.remove(oldStackSize, list_size);
        return QString::number(list_size);
    });

    proc["ЭЛЕМЕНТ"] = static_cast<std::function<QString(QString, QString)>>([=](QString index, QString list)
    {
        int idx = index.toUInt();
        int oldStackSize = m_stack.size();
        m_context = new ProgramContext(removeBrackets(list), m_context);
        evalList();
        ProgramContext *temp = m_context;
        m_context = m_context->parent();
        delete temp;
        int list_size = m_stack.size() - oldStackSize;
        if (list_size >= idx)
            return m_stack.at(oldStackSize + idx);
        m_stack.remove(oldStackSize, list_size);
        return QString();
    });
}

LogoInterpreter::Result LogoInterpreter::infixOp(QString left, LogoInterpreter::Token op, QString right)
{
    bool ok1, ok2;
    int int_left = left.toInt(&ok1);
    int int_right = right.toInt(&ok2);
    if (ok1 && ok2)
    {
//        qDebug() << "integer:" << int_left << op << int_right;
        return infixOp(int_left, op, int_right);
    }

    double double_left = left.toDouble(&ok1);
    double double_right = right.toDouble(&ok2);
    if (ok1 && ok2)
        return infixOp(double_left, op, double_right);

    return Result::Error;
}

LogoInterpreter::Result LogoInterpreter::infixOp(int left, LogoInterpreter::Token op, int right)
{
    Result result;

    static const Result True = Result("1");
    static const Result False = Result("0");

    if (op == "+")
        result = QString::number(left + right);
    else if (op == "-")
        result = QString::number(left - right);
    else if (op == "*")
        result = QString::number(left * right);
    else if (op == "/")
    {
        if (right == 0)
            raiseError("Делить на 0 нельзя!");
        else if (left % right)
            result = QString::number((double)left / (double)right);
        else
            result = QString::number(left / right);
    }
    else if (op == "%")
    {
        if (right == 0)
            raiseError("Делить на 0 нельзя!");
        result = QString::number(left % right);
    }
    else if (op == "^")
        result = QString::number(pow(left, right));
    else if (op == "=")
        result = (left == right)? True: False;
    else if (op == "<>" || op == "!=")
        result = (left != right)? False: True;
    else if (op == "<")
        result = (left < right)? True: False;
    else if (op == ">")
        result = (left > right)? True: False;
    else if (op == "<=")
        result = (left <= right)? True: False;
    else if (op == ">=")
        result = (left >= right)? True: False;

    return result;
}

LogoInterpreter::Result LogoInterpreter::infixOp(double left, LogoInterpreter::Token op, double right)
{
    Result result;

    static const Result True = Result("1");
    static const Result False = Result("0");

    if (op == "+")
        result = QString::number(left + right);
    else if (op == "-")
        result = QString::number(left - right);
    else if (op == "*")
        result = QString::number(left * right);
    else if (op == "/")
        result = QString::number(left / right);
    else if (op == "%")
        result = QString::number(fmod(left, right));
    else if (op == "^")
        result = QString::number(pow(left, right));
    else if (op == "=")
        result = qFuzzyCompare(left, right)? True: False;
    else if (op == "<>" || op == "!=")
        result = qFuzzyCompare(left, right)? False: True;
    else if (op == "<")
        result = (left < right)? True: False;
    else if (op == ">")
        result = (left > right)? True: False;
    else if (op == "<=")
        result = (left <= right)? True: False;
    else if (op == ">=")
        result = (left >= right)? True: False;

    return result;
}


