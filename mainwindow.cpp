#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      mProcessing(false),
      mDebug(false),
      mSpacer(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("Robot");
//    showFullScreen();

    setCursor(QCursor(QPixmap::fromImage(QImage(":/arrow.png")), 1, 4));

    QFile styleFile(":/style.css");
    styleFile.open(QIODevice::ReadOnly);
    QString css = styleFile.readAll();
    css.replace("$color1", "139, 237, 139");
    css.replace("$color2", "139, 227, 237");
    styleFile.close();
    setStyleSheet(css);   

    context = nullptr;

//    #ifdef Q_OS_WIN
//    joy3D = new SpaceMouse(this);
//    #endif

//    imu = new QRotationSensor(this);
//    imu->connectToBackend();
//    imu->start();

    onbvs = new ObjnetVirtualServer(this);
    onbvs->setEnabled(true);
    onbvi = new ObjnetVirtualInterface("main", "127.0.0.1");
    oviMaster = new ObjnetMaster(onbvi);
    oviMaster->setName("main");
    onbvi->setActive(true);

    device = new Robot();
    oviMaster->registerDevice(device, 13);

    connLed = new Led(QColor(139, 237, 139));
    connLed->setFixedSize(50, 50);
    ui->statusbar->addWidget(connLed);
    connlabel = new QLabel("disconnected");
    ui->statusbar->addWidget(connlabel);

    QPushButton *stopBtn = new QPushButton("STOP");
    connect(stopBtn, &QPushButton::clicked, this, &MainWindow::stop);

    enableBtn = new QPushButton("Enable");
    enableBtn->setCheckable(true);
    connect(enableBtn, &QPushButton::toggled, [=](bool checked)
    {
        device->setEnabled(checked);
    });

    joy = new JoystickWidget;
    joy->setColor(QColor(127, 185, 187));
//    joy->setRadius(0.1, 0.23);
    joy->setMinimumSize(300, 300);
    joy->resize(600, 600);


    mProgramLayout = new QVBoxLayout;


    editor = new CodeEditor;
    editor->setMinimumWidth(400);
    editor->viewport()->setCursor(QCursor(QPixmap::fromImage(QImage(":/ibeam.png"))));
    editor->setCursorWidth(3);

    QPushButton *btnPlay = new QPushButton("RUN");
    btnPlay->setShortcut(QKeySequence("Ctrl+R"));
    connect(btnPlay, &QPushButton::clicked, this, &MainWindow::run);

    QPushButton *btnSave = new QPushButton("Save");
    btnSave->setStyleSheet("min-width: 4em;");
    btnSave->setShortcut(QKeySequence("Ctrl+S"));
    connect(btnSave, &QPushButton::clicked, this, &MainWindow::save);

    QPushButton *btnClose = new QPushButton("X");//"×");
    btnClose->setObjectName("closeButton");
    connect(btnClose, &QPushButton::clicked, this, &MainWindow::close);


    stack = new QStackedWidget;
    stack->addWidget(editor);

    QHBoxLayout *btnLay = new QHBoxLayout;
    btnLay->addWidget(btnSave);
//    btnLay->addStretch(1);
    btnLay->addWidget(btnClose);

    QVBoxLayout *controlLay = new QVBoxLayout;
    controlLay->addWidget(btnPlay);
    controlLay->addWidget(stopBtn);
    controlLay->addWidget(joy, 1);
    controlLay->addWidget(enableBtn);

    QGridLayout *lay = new QGridLayout;
    lay->setSpacing(12);
    lay->addLayout(mProgramLayout, 0, 0, 2, 1);
    lay->addLayout(btnLay, 0, 2);
    lay->addWidget(stack, 0, 1, 2, 1);
    lay->addLayout(controlLay, 1, 2);

    ui->centralwidget->setLayout(lay);


    createProcedures();


    listPrograms();

    connect(device, &Robot::commandCompleted, [=]()
    {
        mProcessing = false;
//        qDebug() << "poimal";
    });

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(onTimer()));
    timer->start(16);
}

MainWindow::~MainWindow()
{
//    imu->stop();
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
    stop();
    save();
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if ((e->modifiers() & Qt::ControlModifier))
    {
//        if (e->key() == Qt::Key_R)
//            run();
//        else if (e->key() == Qt::Key_S)
//            save();
    }
    else if (e->key() == Qt::Key_F5)
    {
        if (!isRunning())
            run();
        else
            stop();
    }
    else if (e->key() == Qt::Key_F10)
    {
        step();
        mDebug = true;
    }

}

int MainWindow::opPriority(QString op)
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

void MainWindow::onTimer()
{
    float dt = 0.016f;

    if (context && !mProcessing && !mDebug)
    {
//        mProcessing = true;
        step();
    }

//#ifdef Q_OS_WIN
//    joy3D->read();
//#endif

//#ifdef Q_OS_WIN
//    if (joy3D->isPresent())
//    {
//        SpaceMouse::AxesValues axes = joy3D->getAxesValues();

//    }
//#endif

//    if (imu->isActive())
//    {
//        QRotationReading *r = imu->reading();
//        dwx = qBound(-0.1, r->x() - x[3], 0.1);
//        dwy = qBound(-0.1, r->y() - x[4], 0.1);
//        dwz = qBound(-0.1, r->z() - x[5], 0.1);
//    }

    device->update();

//    if (joy->isActive())
    if (!isRunning())
    {
        float v = joy->y();
        float w = joy->x();// * 10;
        device->setControl(v*3, w*3);
    }

    bool conn = device->isValid() && device->isPresent();
    connLed->setState(conn);
    if (conn)
        connlabel->setText("connected");
    else
        connlabel->setText("disconnected");
}

void MainWindow::run()
{
    editor->setReadOnly(true);
    for (QPushButton *btn: mProgramBtns)
        btn->setEnabled(false);
    QString text = editor->toPlainText();
    mScript = text.split('\n');
    mDebug = false;
    if (!device->isEnabled())
        enableBtn->setChecked(true);
    mStack.clear();
    context = new ScriptContext(text, context);
}

QString MainWindow::evalExpr(QString expr)
{
    QString result;
    ScriptContext *oldContext = context;
    context = new ScriptContext(expr, context);
    do
    {
        result = eval(context->nextToken(), true);
        if (result.isEmpty())
            break;
        mStack << result;
    } while (!context->atEnd());
    delete context;
    context = oldContext;
    return result;
}

QString MainWindow::eval(QString token, bool waitOperand, bool dontTestInfix)
{
    token = token.replace("Ё", "Е");

    qDebug() << "EARLY EVAL" << token << "WAITOP" << waitOperand << "DONT TEST" << dontTestInfix;

    QString infixOp = context->testInfixOp();
    if (!dontTestInfix && !infixOp.isEmpty() && !context->isInfixOp(token))
    {
        QString lastOp = mStack.last();
//        QString lastOp = context->mLastOp;
        qDebug() << "infix" << infixOp << "lastOp" << lastOp;
        if (opPriority(infixOp) > opPriority(lastOp))
        {
            QString param = token;
            token = context->nextToken();
//            context->mLastOp = token;
            QString value = eval(param, true, true);
            mStack.push(value);
            waitOperand = false;
        }
    }

    if (token == "-" && waitOperand)
        token = "~"; // unary minus

    qDebug() << "EVAL" << token << mStack;

    if (proc.contains(token))
    {
        mStack.push(token);
//        context->mLastOp = token;
        QStringList params;
        int pcnt = proc[token].paramCount();
        for (int i=params.count(); i<pcnt; i++)
        {
            QString p = context->nextToken();
            if (token == "ПОВТОР" && i == 1)
                params << p.mid(1).chopped(1);
            else
                params << eval(p, true);
//            context->mLastOp = "";
        }
        qDebug() << "CALL" << token << params;
        QString result = proc[token](params);
        mStack.pop();
        return result;
    }
    if (token == "~")
    {
        mStack.push(token);
//        context->mLastOp = "~"; // unary minus
        QString value = eval(context->nextToken(), true, true);
        mStack.pop();
        return QString::number(-value.toDouble());
    }
    else if (context->isInfixOp(token))
    {
        if (mStack.isEmpty())
        {
            qCritical("JOPPA");
            return "0";
        }
//        context->mLastOp = token;
        QString param1 = mStack.pop();
        mStack.push(token);
        QString param2 = eval(context->nextToken(), true);
        while (opPriority(context->testInfixOp()) > opPriority(token))
        {
            mStack.push(param2);
            param2 = eval(context->nextToken());
        }

        qDebug() << param1 << token << param2;
        QString result;
        if (token == "+")
            result = QString::number(param1.toDouble() + param2.toDouble());
        else if (token == "-")
            result = QString::number(param1.toDouble() - param2.toDouble());
        else if (token == "*")
            result = QString::number(param1.toDouble() * param2.toDouble());
        else if (token == "/")
            result = QString::number(param1.toDouble() / param2.toDouble());
        else if (token == "^")
            result = QString::number(pow(param1.toDouble(), param2.toDouble()));
        else if (token == "=")
            result = qFuzzyCompare(param1.toDouble(), param2.toDouble())? "1": "0";
        else if (token == "<>")
            result = qFuzzyCompare(param1.toDouble(), param2.toDouble())? "0": "1";
        else if (token == "<")
            result = (param1.toDouble() < param2.toDouble())? "1": "0";
        else if (token == ">")
            result = (param1.toDouble() > param2.toDouble())? "1": "0";
        else if (token == "<=")
            result = (param1.toDouble() <= param2.toDouble())? "1": "0";
        else if (token == ">=")
            result = (param1.toDouble() >= param2.toDouble())? "1": "0";

        mStack.pop();

        while ((opPriority(context->testInfixOp()) <= opPriority(token)) && (opPriority(context->testInfixOp()) >= opPriority(mStack.last())))
        {
            qDebug() << "PROVERKA:" << "next" << context->testInfixOp() << "cur" << token << "prev" << mStack.last();
            if (context->testInfixOp().isEmpty())
                break;
            mStack.push(result);
            result = eval(context->nextToken());
        }
        return result;
    }
    else if (token.startsWith("\""))
    {
        return token.mid(1);
    }
    else if (token.startsWith(":"))
    {
        return var(token.mid(1));
    }
    else if (token.indexOf(QRegExp("^[\\d.,]+$")) >= 0)
    {
        return token.replace(",", ".");
    }
    else if (token.startsWith("(") && token.endsWith(")"))
    {
        QString expr = token.mid(1).chopped(1);
        return evalExpr(expr);
    }

    return token; // x3 40 delat, pust budet token
}

void MainWindow::step()
{
    if (!isRunning())
    {
        run();
//        return;
    }

//    QString token = context->testNextToken();

//    if (token.isEmpty())
//    {
//        ScriptContext *oldContext = context;
//        context = context->parent();
//        delete oldContext;
//        if (!context)
//            stop();
//    }

   // qDebug() << "RUN" << token;

    if (context)
    {
        QString result;
//        do
//        {
//            result = eval(context->nextToken());
//            if (result.isEmpty())
//                break;
//            mStack << result;
//            qDebug() << "stack:" << mStack;
//        } while (!context->atEnd());

        result = eval(context->nextToken());

        if (context->atEnd())
        {
            ScriptContext *oldContext = context;
            context = context->parent();
            delete oldContext;
            if (!context)
                stop();
        }
//        QString result = eval(token);
//        qDebug() << "result =" << result;

    }
}

void MainWindow::stop()
{
    mProcessing = false;
    while (context)
    {
        ScriptContext *temp = context;
        context = context->parent();
        delete temp;
    }
    editor->highlightLine(-1);
    device->stop();
    enableBtn->setChecked(false);
    editor->setReadOnly(false);
    for (QPushButton *btn: mProgramBtns)
        btn->setEnabled(true);
}

void MainWindow::listPrograms()
{
    QDir dir;
    if (!dir.exists("programs"))
        dir.mkdir("programs");
    dir.cd("programs");
    QFile mainFile("programs/main.txt");
    if (!mainFile.exists())
    {
        mainFile.open(QIODevice::WriteOnly);
        mainFile.write("");
        mainFile.close();
    }
    QStringList filters;
    filters << "*.txt";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);

    if (!mSpacer)
        mSpacer = new QLabel;

    for (QPushButton *btn: mProgramBtns)
    {
        mProgramLayout->removeWidget(btn);
        btn->deleteLater();
    }
    mProgramBtns.clear();
    mPrograms.clear();

    for (QFileInfo &file: files)
    {
        QString programName = file.baseName();
        QPushButton *btn = new QPushButton(programName);
        btn->setCheckable(true);
        btn->setAutoExclusive(true);
        connect(btn, &QPushButton::clicked, [=]()
        {
            save();
            load(programName);
        });
        mProgramBtns[programName] = btn;
        mProgramLayout->addWidget(btn);
    }
    if (mProgramBtns.contains(mProgramName))
        mProgramBtns[mProgramName]->setChecked(true);
    mProgramLayout->addWidget(mSpacer, 100);

    if (mProgramName.isEmpty())
        load("main");
}

void MainWindow::save()
{
    QString text = editor->toPlainText();
    QString firstLine = text.split("\n").first().trimmed();
    QRegExp rx("^(\\w+):$", Qt::CaseInsensitive);
    if (firstLine.indexOf(rx) != -1)
    {
        mProgramName = rx.cap(1);
    }

    if (mProgramName.isEmpty())
        mProgramName = "main";
    QString filename = "programs/" + mProgramName + ".txt";
    bool reload = false;
    QFile file(filename);
    if (!file.exists())
        reload = true;
    if (text.isEmpty())
    {
        file.remove();
        mProgramName = "";
        reload = true;
    }
    else
    {
        file.open(QIODevice::WriteOnly);
        file.write(text.toUtf8());
        file.close();
    }
    if (reload)
        listPrograms();
}

void MainWindow::load(QString name)
{
    QString filename = "programs/" + name + ".txt";
    QFile file(filename);
    if (!file.exists())
        return;
    mProgramName = name;
    file.open(QIODevice::ReadOnly);
    QString text = QString::fromUtf8(file.readAll());
    file.close();
    editor->setPlainText(text);
    mProgramBtns[name]->setChecked(true);
}



void MainWindow::createProcedures()
{
    proc["ВПЕРЕД"] = [=](QString value)
    {
        device->forward(value.toFloat());
        mProcessing = true;
//        qDebug() << "forward" << value.toFloat();
    };
    proc["НАЗАД"] = [=](QString value)
    {
        device->backward(value.toFloat());
        mProcessing = true;
    };
    proc["ВЛЕВО"] = [=](QString value)
    {
        device->left(value.toFloat());
        mProcessing = true;
    };
    proc["ВПРАВО"] = [=](QString value)
    {
        device->right(value.toFloat());
        mProcessing = true;
    };
    proc["ПЕРОПОДНЯТЬ"] = [=]()
    {
        device->penUp();
        mProcessing = true;
    };
    proc["ПП"] = proc["ПЕРОПОДНЯТЬ"];
    proc["ПЕРООПУСТИТЬ"] = [=]()
    {
        device->penDown();
        mProcessing = true;
    };
    proc["ПО"] = proc["ПЕРООПУСТИТЬ"];

    proc["ПОВТОР"] = [=](QString count, QString list)
    {
        int cnt = count.toInt();
        for (int i=0; i<cnt; i++)
            context = new ScriptContext(list, context);
    };

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
}
