#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      scene(nullptr),
      turtle(nullptr),
      mSpacer(nullptr),
      mProcessing(false),
      mDebug(false)
{
    ui->setupUi(this);
    setWindowTitle("LogoPashka");
    showFullScreen();

    setCursor(QCursor(QPixmap::fromImage(QImage(":/arrow.png")), 1, 4));

    int font_size = width() / 83;
    QScreen *screen = QApplication::screens().at(0);
    if (screen)
    {
        font_size = screen->size().width() / screen->logicalDotsPerInch();
//        qDebug() << screen->logicalDotsPerInch() << screen->physicalDotsPerInch();
//        qDebug() << screen->physicalSize();
    }
//    qDebug() << font_size;

    QFile styleFile(":/style.css");
    styleFile.open(QIODevice::ReadOnly);
    QString css = styleFile.readAll();
    css.replace("$font_size", QString::number(font_size)+"pt");
    css.replace("$color1", "139, 237, 139");
    css.replace("$color2", "139, 227, 237");
    styleFile.close();
    setStyleSheet(css);

    logo = new LogoInterpreter;
    connect(logo, &LogoInterpreter::procedureFetched, this, &MainWindow::onLogoProcedureFetched);
    connect(logo, &LogoInterpreter::error, this, &MainWindow::onLogoError);

//    #ifdef Q_OS_WIN
//    joy3D = new SpaceMouse(this);
//    #endif

//    imu = new QRotationSensor(this);
//    imu->connectToBackend();
//    imu->start();

#if defined(ONB)
    onbvs = new ObjnetVirtualServer(this);
    onbvs->setEnabled(true);
    onbvi = new ObjnetVirtualInterface("main", "127.0.0.1");
    oviMaster = new ObjnetMaster(onbvi);
    oviMaster->setName("main");
    onbvi->setActive(true);

    device = new Robot();
    oviMaster->registerDevice(device, 13);
#endif

    connLed = new Led(QColor(139, 237, 139));
    connLed->setFixedSize(50, 50);
    ui->statusbar->addWidget(connLed);
    connlabel = new QLabel("disconnected");
    ui->statusbar->addWidget(connlabel);

    QPushButton *stopBtn = new QPushButton("STOP");
    connect(stopBtn, &QPushButton::clicked, this, &MainWindow::stop);

#if defined(ONB)
    enableBtn = new QPushButton("Enable");
    enableBtn->setCheckable(true);
    connect(enableBtn, &QPushButton::toggled, [=](bool checked)
    {
        device->setEnabled(checked);
    });
#endif

    joy = new JoystickWidget;
    joy->setColor(QColor(127, 185, 187));
//    joy->setRadius(0.1, 0.23);
    joy->setMinimumSize(300, 300);
    joy->resize(600, 600);


    mProgramGroup = new QGroupBox();
    mProgramGroup->setStyleSheet("QGroupBox {border: none; margin: 0; padding: 0;}");
    mProgramLayout = new QVBoxLayout;
    mProgramGroup->setLayout(mProgramLayout);


    editor = new CodeEditor;
    editor->setObjectName("editor");
    editor->setMinimumWidth(300);
    editor->viewport()->setCursor(QCursor(QPixmap::fromImage(QImage(":/ibeam.png"))));
    editor->setCursorWidth(3);

//    QScrollBar *bar = new QScrollBar(Qt::Vertical);
//    editor->verticalScrollBar();
//    editor->setVerticalScrollBar(bar);

    console = new QLineEdit;
    connect(console, &QLineEdit::returnPressed, [=]()
    {
        logo->execute(console->text());
    });

    connect(logo, &QThread::finished, [=]()
    {
        if (!logo->isErrorState())
            console->setText(logo->result());
        console->selectAll();
    });

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


    scene = new Scene();
    turtle = scene->turtle();
    logo->setTurtle(turtle);

    QPushButton *btnCamMain = new QPushButton("fly");
    btnCamMain->setStyleSheet("min-width: 100px;");
    btnCamMain->setCheckable(true);
    btnCamMain->setAutoExclusive(true);
    btnCamMain->setChecked(true);
    connect(btnCamMain, &QPushButton::clicked, [=](){scene->setView(Scene::ViewMain);});
    QPushButton *btnCamTop = new QPushButton("top");
    btnCamTop->setStyleSheet("min-width: 100px;");
    btnCamTop->setCheckable(true);
    btnCamTop->setAutoExclusive(true);
    connect(btnCamTop, &QPushButton::clicked, [=](){scene->setView(Scene::ViewTop);});
    QPushButton *btnCamFollow = new QPushButton("follow");
    btnCamFollow->setStyleSheet("min-width: 100px;");
    btnCamFollow->setCheckable(true);
    btnCamFollow->setAutoExclusive(true);
    connect(btnCamFollow, &QPushButton::clicked, [=](){scene->setView(Scene::ViewFollow);});

    sceneBox = new QGroupBox;
    sceneBox->setStyleSheet("QGroupBox {background-color: white; margin: 0;}");
    QVBoxLayout *sceneboxlay = new QVBoxLayout;
    QHBoxLayout *scenebtnlay = new QHBoxLayout;
    sceneboxlay->setContentsMargins(0, 0, 0, 0);
    sceneBox->setLayout(sceneboxlay);
    sceneboxlay->addLayout(scenebtnlay);
    sceneboxlay->addWidget(scene);

    scenebtnlay->addWidget(btnCamMain);
    scenebtnlay->addWidget(btnCamTop);
    scenebtnlay->addWidget(btnCamFollow);

//    stack = new QStackedWidget;
//    stack->addWidget(editor);
//    stack->addWidget(sceneBox);

    QSplitter *splitter = new QSplitter(this);
    splitter->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
    splitter->setOrientation(Qt::Vertical);
//    QWidget *ebox = new QWidget;
//    QHBoxLayout *elay = new QHBoxLayout;
//    ebox->setLayout(elay);
//    elay->addWidget(editor);
//    elay->addWidget(editor->verticalScrollBar());
//    splitter->addWidget(ebox);
    splitter->addWidget(editor);
    splitter->addWidget(sceneBox);

    btnScene = new QPushButton("Model");
    btnScene->setCheckable(true);
    connect(btnScene, &QPushButton::toggled, [=](bool checked)
    {
        editor->setHidden(checked);
        sceneBox->setVisible(checked);
//        if (checked)
//            stack->setCurrentWidget(sceneBox);
//        else
//            stack->setCurrentWidget(editor);
    });

    editor->hide();

    btnPD = new QPushButton("ПО");
    btnPD->setObjectName("penDown");
    btnPD->setCheckable(true);
    btnPD->setAutoExclusive(true);
    connect(btnPD, &QPushButton::clicked, [=]()
    {
        if (turtle)
            turtle->penDown();
    });
    btnPU = new QPushButton("ПП");
    btnPU->setObjectName("penUp");
    btnPU->setFixedWidth(100);
    btnPU->setCheckable(true);
    btnPU->setAutoExclusive(true);
    btnPU->setChecked(true);
    connect(btnPU, &QPushButton::clicked, [=]()
    {
        if (turtle)
            turtle->penUp();
    });
    QGroupBox *penbox = new QGroupBox();
    QHBoxLayout *penlay = new QHBoxLayout;
    penbox->setLayout(penlay);
    penlay->addWidget(btnPD);
    penlay->addWidget(btnPU);

//    stack->setCurrentIndex(1);

    QHBoxLayout *btnLay = new QHBoxLayout;
    btnLay->addWidget(btnSave);
//    btnLay->addStretch(1);
    btnLay->addWidget(btnClose);

    QVBoxLayout *controlLay = new QVBoxLayout;
    controlLay->addWidget(btnPlay);
    controlLay->addWidget(stopBtn);
    controlLay->addWidget(penbox);
    controlLay->addWidget(joy, 1);
    controlLay->addWidget(btnScene);
#if defined(ONB)
    controlLay->addWidget(enableBtn);
#endif

    QHBoxLayout *lay = new QHBoxLayout;
    lay->setContentsMargins(16, 16, 16, 16);
    lay->setSpacing(16);
    lay->addWidget(mProgramGroup);

    QVBoxLayout *mainlay = new QVBoxLayout;
    mainlay->addWidget(splitter);
    mainlay->addWidget(console);
    lay->addLayout(mainlay, 1);

    QVBoxLayout *rightlay = new QVBoxLayout;
    rightlay->addLayout(btnLay);
    rightlay->addLayout(controlLay);
    lay->addLayout(rightlay);

    ui->centralwidget->setLayout(lay);

    listPrograms();

#if defined(ONB)
    connect(device, &Robot::commandCompleted, [=]()
    {
        mProcessing = false;
    });
#endif

//    connect(scene, &Scene::commandCompleted, [=]()
//    {
//        mProcessing = false;
//    });

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(onTimer()));
    timer->start(16);
}

MainWindow::~MainWindow()
{
//    imu->stop();
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *)
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
        if (!isRunning() || mDebug)
            run();
        else
            stop();
    }
    else if (e->key() == Qt::Key_F10)
    {
        setDebugMode(true);
        step();
    }

}



void MainWindow::onTimer()
{
    float dt = 0.016f;

//    if (context && !mProcessing && !mDebug)
//    {
////        mProcessing = true;
//        step();
//    }

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

#if defined(ONB)
    device->update();
#endif

//    if (joy->isActive())
    if (!isRunning())
    {
        float v = joy->y();
        float w = -joy->x();// * 10;
#if defined(ONB)
        device->setControl(v*3, w*3);
#endif
//        if (scene && scene->isVisible())
        if (turtle)
            turtle->setControl(v*3, w*3);
    }

    scene->integrate(dt);

//    if (logo->isRunning())
//    {
//        QColor selColor(139, 237, 139);
//        QColor lineColor(139, 227, 237, 192);
//        editor->clearHighlights();
//        editor->highlightText(logo->lastProcPos(), logo->curPos(), selColor, lineColor);
//        QTextCursor cur = editor->textCursor();
//        cur.setPosition(logo->lastProcPos());
//        editor->setTextCursor(cur);
//        editor->ensureCursorVisible();
//    }

#if defined(ONB)
    bool conn = device->isValid() && device->isPresent();
    connLed->setState(conn);
    if (conn)
        connlabel->setText("connected");
    else
        connlabel->setText("disconnected");
#endif

    scene->update();
    if (scene->isVisible())
        joy->update(); // FUCKING HACK
}

void MainWindow::run()
{
    if (logo->isRunning())
    {
        if (mDebug)
        {
            setDebugMode(false);
            logo->setDebugMode(false);
        }
        return;
    }

    save();
    editor->clearHighlights();
    editor->setReadOnly(true);
    for (QPushButton *btn: mProgramBtns)
        btn->setEnabled(false);
    QString text = editor->toPlainText();
//    if (!mProgramName.isEmpty() && mProgramName != "MAIN")
//        text = mProgramName;
//    setDebugMode(false);
//#if defined(ONB)
//    if (!device->isEnabled())
//        enableBtn->setChecked(true);
//#endif
//    btnScene->setChecked(true);

    logo->execute(text, mDebug);
    connect(logo, &QThread::finished, this, &MainWindow::stop);
    qDebug() << "*** RUN ***";
}

//QString MainWindow::evalExpr(QString expr)
//{
//    QString result;
//    ProgramContext *oldContext = context;
//    context = new ProgramContext(expr, context);
//    do
//    {
//        result = eval(context->nextToken(), true);
//        if (result.isEmpty())
//            break;
//        mStack << result;
//    } while (!context->atEnd());
//    delete context;
//    context = oldContext;
//    return result;
//}

//QString MainWindow::eval(QString token, bool waitOperand, bool dontTestInfix)
//{
//    token = token.replace("Ё", "Е");

////    qDebug() << "EARLY EVAL" << token << "WAITOP" << waitOperand << "DONT TEST" << dontTestInfix;

//    if (proc.contains(token))
//        dontTestInfix = true;

//    QString infixOp = context->testInfixOp();
//    if (!dontTestInfix && !infixOp.isEmpty() && !context->isInfixOp(token))
//    {
//        if (mStack.isEmpty())
//            mStack.append(""); // empty operator ??
//        QString lastOp;
////        if (mStack.isEmpty())
//            lastOp = mStack.last();
////        QString lastOp = context->mLastOp;
////        qDebug() << "infix" << infixOp << "lastOp" << lastOp;
//        if (opPriority(infixOp) > opPriority(lastOp))
//        {
//            QString param = token;
//            token = context->nextToken();
////            context->mLastOp = token;
//            QString value = eval(param, true, true);
//            mStack.push(value);
//            waitOperand = false;
//        }
//    }

//    if (token == "-" && waitOperand)
//        token = "~"; // unary minus

//    qDebug() << "EVAL" << token << mStack;

//    if (mPrograms.contains(token) && !proc.contains(token))
//    {
//        QStringList params;
//        params << token;
//        proc["ЗАГРУЗИТЬ"](params);
//    }

//    if (proc.contains(token))
//    {
//        mStack.push(token);
////        context->mLastOp = token;
//        QStringList params;
//        int pcnt = proc[token].paramCount();
//        for (int i=params.count(); i<pcnt; i++)
//        {
//            QString p = context->nextToken();
//            if ((token == "ПОВТОР" || token == "ЕСЛИ" || token == "ЕСЛИИНАЧЕ") && i > 0)
//            {
//                if (p.startsWith("("))
//                    p = p.mid(1);
//                if (p.endsWith(")"))
//                    p = p.left(p.size()-1);//chopped(1);
//                params << p;
//            }
//            else
//                params << eval(p, true);
////            context->mLastOp = "";
//        }
////        QStringList params;
////        for (int i=0; i<pcnt; i++)
////            params << mStack.pop();
//        qDebug() << ">>" << token << params;
//        QString result = proc[token](params);
//        qDebug() << "<<" << result;
//        mStack.pop();
//        return result;
//    }
//    if (token == "~")
//    {
//        mStack.push(token);
////        context->mLastOp = "~"; // unary minus
//        QString value = eval(context->nextToken(), true, true);
//        mStack.pop();
//        return QString::number(-value.toDouble());
//    }
//    else if (context->isInfixOp(token))
//    {
//        if (mStack.isEmpty())
//        {
//            qCritical("JOPPA");
//            return "0";
//        }
////        context->mLastOp = token;
//        QString param1 = mStack.pop();
//        mStack.push(token);
//        QString param2 = eval(context->nextToken(), true);
//        while (opPriority(context->testInfixOp()) > opPriority(token))
//        {
//            mStack.push(param2);
//            param2 = eval(context->nextToken());
//        }

////        qDebug() << param1 << token << param2;
//        QString result;
//        if (token == "+")
//            result = QString::number(param1.toDouble() + param2.toDouble());
//        else if (token == "-")
//            result = QString::number(param1.toDouble() - param2.toDouble());
//        else if (token == "*")
//            result = QString::number(param1.toDouble() * param2.toDouble());
//        else if (token == "/")
//            result = QString::number(param1.toDouble() / param2.toDouble());
//        else if (token == "^")
//            result = QString::number(pow(param1.toDouble(), param2.toDouble()));
//        else if (token == "=")
//            result = qFuzzyCompare(param1.toDouble(), param2.toDouble())? "1": "0";
//        else if (token == "<>")
//            result = qFuzzyCompare(param1.toDouble(), param2.toDouble())? "0": "1";
//        else if (token == "<")
//            result = (param1.toDouble() < param2.toDouble())? "1": "0";
//        else if (token == ">")
//            result = (param1.toDouble() > param2.toDouble())? "1": "0";
//        else if (token == "<=")
//            result = (param1.toDouble() <= param2.toDouble())? "1": "0";
//        else if (token == ">=")
//            result = (param1.toDouble() >= param2.toDouble())? "1": "0";

//        qDebug() << param1 << token << param2 << "=" << result;

//        mStack.pop();

//        while ((opPriority(context->testInfixOp()) <= opPriority(token)) && (opPriority(context->testInfixOp()) > opPriority(mStack.last())))
//        {
////            qDebug() << "PROVERKA:" << "next" << context->testInfixOp() << "cur" << token << "prev" << mStack.last();
//            if (context->testInfixOp().isEmpty())
//                break;
//            mStack.push(result);
//            result = eval(context->nextToken());
//        }
//        return result;
//    }
//    else if (token.startsWith("\""))
//    {
//        return token.mid(1);
//    }
//    else if (token.startsWith(":"))
//    {
//        QString varname = token.mid(1);
//        QString value = context->var(varname);
//        if (!value.isNull())
//            return value;
//        return var(varname);
//    }
//    else if (token.indexOf(QRegExp("^[\\d.,]+$")) >= 0)
//    {
//        return token.replace(",", ".");
//    }
//    else if (token.startsWith("(") && token.endsWith(")"))
//    {
//        QString expr = token.mid(1, token.size()-2);//.chopped(1);
//        evalExpr(expr);
//        return mStack.pop();
//    }

//    return token; // x3 40 delat, pust budet token
//}

void MainWindow::step()
{
    if (!isRunning())
    {
        setDebugMode(true);
        run();
    }
    else
    {
        logo->setDebugMode(true);
        logo->doDebugStep();
    }
}

void MainWindow::stop()
{
    disconnect(logo, &QThread::finished, this, &MainWindow::stop);
    logo->stop();
    logo->wait(1000);
    logo->terminate(); // hard stop
    mProcessing = false;

    turtle->stop();

    if (!logo->isErrorState())
    {
        setDebugMode(false);
        editor->clearHighlights();
    }
    else
    {
        setDebugMode(true);
    }

#if defined(ONB)
    device->stop();
    enableBtn->setChecked(false);
#endif

    editor->setReadOnly(false);
    for (QPushButton *btn: mProgramBtns)
        btn->setEnabled(true);
    qDebug() << "*** STOP ***";
}

void MainWindow::onLogoProcedureFetched(int start, int end)
{
    QColor selColor(139, 237, 139);
    QColor lineColor(139, 227, 237, 192);
    editor->clearHighlights();
    editor->highlightText(start, end, selColor, lineColor);
    QTextCursor cur = editor->textCursor();
    cur.setPosition(start);
    editor->setTextCursor(cur);
    editor->ensureCursorVisible();
}

void MainWindow::onLogoError(int start, int end, QString reason)
{
    disconnect(logo, &LogoInterpreter::error, this, &MainWindow::onLogoError);
    if (mPrograms.contains(logo->programName()))
    {
        open(logo->programName());
    }

    editor->clearHighlights();
    editor->highlightText(start, end, Qt::red, QColor(255, 0, 0, 64));
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
        QString programName = file.baseName().toUpper();
        QPushButton *btn = new QPushButton(file.baseName());
        btn->setCheckable(true);

        btn->setAutoExclusive(true);
        connect(btn, &QPushButton::clicked, [=]()
        {
            save();
            open(programName);
            editor->show();
//            sceneBox->hide();
            btnScene->setChecked(false);
        });
        mPrograms << programName.toUpper();
        mProgramBtns[programName] = btn;
        mProgramLayout->addWidget(btn);

//        QStringList params;
//        params << programName;
//        proc["ЗАГРУЗИТЬ"](params);
    }
    if (mProgramBtns.contains(mProgramName))
        mProgramBtns[mProgramName]->setChecked(true);
    mProgramLayout->addWidget(mSpacer, 100);

    if (mProgramName.isEmpty())
        open("MAIN");
}

void MainWindow::save()
{
    QString text = editor->toPlainText();
    QString firstLine = text.split("\n").first().trimmed();
    QRegExp rx("^ЭТО\\s+(\\w+)", Qt::CaseInsensitive);
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

void MainWindow::open(QString name)
{
    QString filename = "programs/" + name + ".txt";
    if (!QFile::exists(filename))
        return;
    mProgramName = name;
    QString text = load(name);
    editor->setPlainText(text);
    mProgramBtns[name]->setChecked(true);
}

QString MainWindow::load(QString name)
{
    QString filename = "programs/" + name + ".txt";
    QFile file(filename);
    if (!file.exists())
        return QString();
    file.open(QIODevice::ReadOnly);
    QString text = QString::fromUtf8(file.readAll());
    file.close();
    return text;
}

void MainWindow::setDebugMode(bool enabled)
{
    mDebug = enabled;
    if (mDebug)
    {
        editor->show();
        sceneBox->show();
    }
    else
    {
        editor->hide();
        sceneBox->show();
    }
}



