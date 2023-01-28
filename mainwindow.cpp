#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      scene(nullptr),
      turtle(nullptr),
      mProcessing(false),
      mDebug(false)
{
    ui->setupUi(this);
    setWindowTitle("LogoPashka");
    bool fullscreen = true;

    setCursor(QCursor(QPixmap::fromImage(QImage(":/arrow.png")), 1, 4));

    int font_size = width() / 83;
    QScreen *screen = QApplication::screens().at(0);
    if (screen)
    {
        font_size = screen->size().width() / screen->logicalDotsPerInch();
//        qDebug() << screen->logicalDotsPerInch() << screen->physicalDotsPerInch();
//        qDebug() << screen->physicalSize();
        if (screen->geometry().width() < screen->geometry().height())
            fullscreen = false;
    }
//    qDebug() << font_size;

    if (fullscreen)
        showFullScreen();
//    else
//    {
//        move(0, 0);
//        resize(screen->geometry().width(), height());
//    }

    QFile styleFile(":/style.css");
    styleFile.open(QIODevice::ReadOnly);
    QString css = styleFile.readAll();
    css.replace("$font_size", QString::number(font_size)+"pt");
    css.replace("$color1", "139, 237, 139");
    css.replace("$color2", "139, 227, 237");
    styleFile.close();
    setStyleSheet(css);

    logo = new LogoInterpreter;
    connect(logo, &LogoInterpreter::finished, this, &MainWindow::stop);
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

    device = new OnbTurtle();
    oviMaster->registerDevice(device, 13);
#endif

    connLed = new Led(QColor(139, 237, 139));
    connLed->setFixedSize(60, 60);
    ui->statusbar->addWidget(connLed);
    connlabel = new QLabel("");
    ui->statusbar->addWidget(connlabel);

    enableBtn = new QPushButton("Включить");
    enableBtn->setCheckable(true);
    connect(enableBtn, &QPushButton::toggled, [=](bool checked)
    {
#if defined(ONB)
        device->setEnabled(checked);
#endif
    });
    enableBtn->hide();

    joy = new JoystickWidget;
    joy->setColor(QColor(127, 185, 187));
    joy->setMinimumSize(font_size * 20, font_size * 20);
//    joy->setRadius(0.1, 0.23);
//    joy->setMinimumSize(300, 300);
//    joy->resize(600, 600);


    mProgramListView = new QListView();
    mProgramListView->setMinimumWidth(width() / 6);
    mProgramListModel = new QStringListModel();
    mProgramListView->setModel(mProgramListModel);
//    mProgramListView->setViewMode(QListView::IconMode);
//    mProgramListView->setMovement(QListView::Static);
//    mProgramListView->setItemAlignment(Qt::AlignHCenter);
    mProgramListView->setFocusPolicy(Qt::NoFocus);
//    mProgramListView->setVerticalScrollMode(QListView::ScrollPerPixel);
//    mProgramListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QPushButton *btnAddProgram = new QPushButton("+");
    btnAddProgram->setObjectName("addButton");
//    QGraphicsDropShadowEffect *shadow = new QGraphicsDropShadowEffect;
//    shadow->setBlurRadius(font_size * 2);
//    shadow->setOffset(0, 0);
//    shadow->setColor(Qt::white);
//    btnAddProgram->setGraphicsEffect(shadow);

//    QPushButton *btnDelProgram = new QPushButton("X");
//    btnDelProgram->setObjectName("delButton");

    QLineEdit *editAddProgram = new QLineEdit;
    editAddProgram->setPlaceholderText("Имя программы");
    editAddProgram->hide();

    QWidget *progw = new QWidget;
    QVBoxLayout *play = new QVBoxLayout;
    play->setContentsMargins(0, 0, 0, 0);
    play->setSpacing(0);
    progw->setLayout(play);
    QHBoxLayout *phlay = new QHBoxLayout;
    phlay->setAlignment(Qt::AlignLeft);
    play->addLayout(phlay, 0);
    phlay->addWidget(editAddProgram, 1);
    phlay->addWidget(btnAddProgram, 0);
//    phlay->addWidget(btnDelProgram, 0);
    play->addWidget(mProgramListView, 1);

    connect(btnAddProgram, &QPushButton::clicked, [=]()
    {
        if (!editAddProgram->isVisible())
        {
            editAddProgram->show();
            editAddProgram->setFocus();
            btnAddProgram->setText("OK");
        }
        else
        {
            QString name = editAddProgram->text();
            bool success = false;
            if (name.isEmpty())
                success = true;
            else if (mPrograms.contains(name))
                success = false;
            else
            {
                QFile file(path(name));
                if (file.open(QIODevice::WriteOnly))
                {
                    success = true;
                    save(); // current program
                    mProgramName = name;
                    listPrograms(); // and open new empty program
                }
                file.close();
            }

            if (success)
            {
                editAddProgram->hide();
                editAddProgram->clear();
                btnAddProgram->setText("+");
            }
        }
    });

    connect(editAddProgram, &QLineEdit::returnPressed, [=]()
    {
        btnAddProgram->click();
    });

    connect(editAddProgram, &QLineEdit::textChanged, [=]()
    {
        if (editAddProgram->isVisible())
        {
            QString name = editAddProgram->text();
            btnAddProgram->setDisabled(name.isEmpty() || mPrograms.contains(name));
        }
    });


    mCommandListView = new QListView();
    mCommandListView->setMinimumWidth(width() / 6);
    mCommandListModel = new QStringListModel();
    mCommandListView->setModel(mCommandListModel);
//    mCommandListView->setViewMode(QListView::IconMode);
//    mCommandListView->setMovement(QListView::Static);
//    mCommandListView->setItemAlignment(Qt::AlignHCenter);
    mCommandListView->setFocusPolicy(Qt::NoFocus);
    mCommandListView->setEditTriggers(QListView::NoEditTriggers);
//    mCommandListView->setVerticalScrollMode(QListView::ScrollPerPixel);
//    mCommandListView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);


    editor = new CodeEditor;
    editor->setObjectName("editor");
    editor->setMinimumWidth(300);
    editor->viewport()->setCursor(QCursor(QPixmap::fromImage(QImage(":/ibeam.png"))));
    editor->setCursorWidth(3);


    console = new ConsoleEdit;
    console->setObjectName("console");
    connect(console, &ConsoleEdit::returnPressed, [=]()
    {
        if (console->isReadOnly())
            return;
        if (console->text().isEmpty())
            return;

        save();
        console->setReadOnly(true);
        editor->setReadOnly(true);
        if (enableBtn->isVisible())
        {
            enableBtn->setChecked(true);
        }
        else
        {
            showScene();
        }
        mProgramListView->setEnabled(false);
//        m_btnRun->setEnabled(false);
        logo->execute(console->text(), "#console");
    });

    connect(mCommandListView, &QListView::clicked, [=](const QModelIndex &idx)
    {
        QString cmd = idx.data().toString();
        int paramCount = logo->procInfo(cmd).paramCount();
        QString text = cmd;
        int pos = cmd.size() + 1;
        for (int i=0; i<paramCount; i++)
            text += " ?";
//        console->setCursorPosition(console->text().size());
//        console->insert(text);
        console->setText(text);
        console->setCursorPosition(pos);
        if (paramCount)
            console->setSelection(pos, 1);
        console->setFocus();
    });

    connect(mCommandListView, &QListView::doubleClicked, [=](const QModelIndex &idx)
    {
//        save(); // make errors
        QString cmd = idx.data().toString();
        const LogoProcedure &proc = logo->procInfo(cmd);
        bool isNative = proc.isNative();
        if (isNative)
        {
            mProgramName = "";
            QString text = ";Встроенная команда: " + cmd + "\n";
            QStringList aliases = proc.aliases();
            if (!aliases.isEmpty())
                text += ";Синонимы: " + aliases.join(", ") + "\n";
            if (proc.textOffset())
            {
                text += "\n;";
                QString desc = m_nativeCommandsText.mid(proc.textOffset(), proc.textLength());
                desc = desc.replace(QRegExp("\n; ?"), "\n");
                text += desc;
            }
            editor->clear();
            editor->insertPlainText(text);
            QTextCursor cur = editor->textCursor();
            cur.setPosition(0);
            editor->setTextCursor(cur);
            editor->show();
//            editor->setReadOnly(true);
        }
        else
        {
            QString programName = proc.programName();
            if (!programName.isEmpty())
            {
                int offset = proc.textOffset();
                open(programName);
                QTextCursor cur = editor->textCursor();
                cur.setPosition(offset);
                editor->setTextCursor(cur);
                editor->centerCursor();
                editor->highlightText(proc.textOffset(), proc.textOffset() + proc.textLength(), Qt::white, Qt::white);
//                editor->highlightCurrentLine();
            }
        }
    });

    m_btnRun = new QPushButton("ПУСК");
    m_btnRun->setShortcut(QKeySequence("Ctrl+R"));
    connect(m_btnRun, &QPushButton::clicked, this, &MainWindow::run);

    m_btnStop = new QPushButton("СТОП");
    connect(m_btnStop, &QPushButton::clicked, this, &MainWindow::stop);

    m_btnSave = new QPushButton("Сохранить");
    m_btnSave->setStyleSheet("min-width: 4em;");
    m_btnSave->setShortcut(QKeySequence("Ctrl+S"));
    connect(m_btnSave, &QPushButton::clicked, this, &MainWindow::save);

    m_btnClose = new QPushButton("X");//"×");
    m_btnClose->setObjectName("closeButton");
    connect(m_btnClose, &QPushButton::clicked, this, &MainWindow::close);


    scene = new Scene();
    turtle = scene->turtle();
    logo->setTurtle(turtle);

    m_btnCamMain = new QPushButton("Вид сбоку");
    m_btnCamMain->setCheckable(true);
    m_btnCamMain->setAutoExclusive(true);
    m_btnCamMain->setChecked(true);
    connect(m_btnCamMain, &QPushButton::clicked, [=](){scene->setView(Scene::ViewMain);});
    m_btnCamTop = new QPushButton("Вид сверху");
    m_btnCamTop->setCheckable(true);
    m_btnCamTop->setAutoExclusive(true);
    connect(m_btnCamTop, &QPushButton::clicked, [=](){scene->setView(Scene::ViewTop);});
    m_btnCamFollow = new QPushButton("Наблюдать");
    m_btnCamFollow->setCheckable(true);
    m_btnCamFollow->setAutoExclusive(true);
    connect(m_btnCamFollow, &QPushButton::clicked, [=](){scene->setView(Scene::ViewFollow);});
    m_btnCamChase = new QPushButton("Следовать");
    m_btnCamChase->setCheckable(true);
    m_btnCamChase->setAutoExclusive(true);
    connect(m_btnCamChase, &QPushButton::clicked, [=](){scene->setView(Scene::ViewChase);});

    sceneBox = new QGroupBox;
    sceneBox->setStyleSheet("QGroupBox {background-color: white; margin: 0;}");
    QVBoxLayout *sceneboxlay = new QVBoxLayout;
    QHBoxLayout *scenebtnlay = new QHBoxLayout;
    sceneboxlay->setContentsMargins(0, 0, 0, 0);
    sceneBox->setLayout(sceneboxlay);
    sceneboxlay->addLayout(scenebtnlay);
    sceneboxlay->addWidget(scene);

    scenebtnlay->addWidget(m_btnCamMain);
    scenebtnlay->addWidget(m_btnCamTop);
    scenebtnlay->addWidget(m_btnCamFollow);
    scenebtnlay->addWidget(m_btnCamChase);

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

    btnScene = new QPushButton("3D модель");
    btnScene->setCheckable(true);
    connect(btnScene, &QPushButton::toggled, [=](bool checked)
    {
        editor->setHidden(checked);
        sceneBox->setVisible(checked);
    });

#if defined(ONB)
    connect(oviMaster, &ObjnetMaster::devConnected, [=](uint8_t addr)
    {
        if (addr == device->netAddress() && device->name() == "SpeedCtl")
        {
            btnScene->hide();
            enableBtn->show();
            turtle = device;
            logo->setTurtle(turtle);
            sceneBox->hide();
            editor->show();
        }
    });

    connect(oviMaster, &ObjnetMaster::devDisconnected, [=](uint8_t addr)
    {
        if (addr == device->netAddress())
        {
            enableBtn->hide();
            btnScene->show();
            turtle = scene->turtle();
            logo->setTurtle(turtle);
            sceneBox->show();
            editor->hide();
        }
    });
#endif

    editor->hide();

    btnPD = new QPushButton("ПО");
    btnPD->setObjectName("penDown");
    btnPD->setCheckable(true);
    connect(btnPD, &QPushButton::clicked, [this]()
    {
        if (turtle)
            turtle->penDown();
    });
    btnPU = new QPushButton("ПП");
    btnPU->setObjectName("penUp");
//    btnPU->setFixedWidth(100);
    btnPU->setCheckable(true);
    connect(btnPU, &QPushButton::clicked, [this]()
    {
        if (turtle)
            turtle->penUp();
    });
    QWidget *penbox = new QWidget();
    QHBoxLayout *penlay = new QHBoxLayout;
    penlay->setContentsMargins(0, 0, 0, 0);
    penbox->setLayout(penlay);
    penlay->addWidget(btnPD);
    penlay->addWidget(btnPU);

//    stack->setCurrentIndex(1);

    QHBoxLayout *btnLay = new QHBoxLayout;
    btnLay->addWidget(m_btnSave);
//    btnLay->addStretch(1);
    btnLay->addWidget(m_btnClose);

    QVBoxLayout *controlLay = new QVBoxLayout;
    controlLay->addWidget(m_btnRun);
    controlLay->addWidget(m_btnStop);
    controlLay->addStretch(1);
    controlLay->addWidget(penbox);
    controlLay->addSpacing(font_size * 2);
    controlLay->addWidget(joy);
    controlLay->addSpacing(font_size * 2);
    controlLay->addWidget(btnScene);
    controlLay->addWidget(enableBtn);

    QTabWidget *mLeftWidget = new QTabWidget;
    mLeftWidget->addTab(progw, "Программы");
    mLeftWidget->addTab(mCommandListView, "Команды");

    connect(mLeftWidget, &QTabWidget::currentChanged, [=](int index)
    {
        if (mCommandListView->isVisible())
        {
            updateCommands();
        }
    });

    QHBoxLayout *lay = new QHBoxLayout;
    lay->setContentsMargins(16, 16, 16, 16);
    lay->setSpacing(16);
//    lay->addWidget(mProgramGroup);
//    lay->addWidget(mProgramListView);
    lay->addWidget(mLeftWidget);

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
    updateCommands();

    connect(mProgramListView, &QListView::clicked, [=](const QModelIndex &idx)
    {
        QString programName = idx.data().toString();
        if (programName != mProgramName)
            save();
        open(programName);
//        editor->show();
//            sceneBox->hide();
        btnScene->setChecked(false);
    });

    connect(mProgramListModel, &QStringListModel::dataChanged, [=](const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
    {
        int idx = topLeft.row();
        if (idx >= 0 && idx < mPrograms.size())
        {
            bool success = true;
            QString oldname = mPrograms.at(idx);
            QString newname = topLeft.data().toString();
            if (newname.isEmpty() || oldname == newname || mPrograms.contains(newname))
                success = false;
            else
                success = QFile::rename(path(oldname), path(newname));

            if (success)
            {
                mPrograms[idx] = newname;
                if (mProgramName == oldname)
                    mProgramName = newname;
                listPrograms();
            }
            else
                mProgramListModel->setData(topLeft, oldname);
        }
    });

    // read description of native commands:
    QFile f(":/proc.txt");
    f.open(QIODevice::ReadOnly);
    m_nativeCommandsText = f.readAll();
    f.close();
    logo->extractProcedures(m_nativeCommandsText, "");

    QTimer *timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), SLOT(onTimer()));
    timer->start(16);

    QGamepadManager *gpm = QGamepadManager::instance();
    connect(gpm, &QGamepadManager::connectedGamepadsChanged, [=]()
    {
        qDebug() << "Gamepads:" << gpm->connectedGamepads();
//        if (gpm->connectedGamepads().isEmpty())
//        {
//            joy->show();
//        }
//        else
//        {
//            joy->hide();
//        }
    });

    gamepad = new QGamepad(0, this);
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

//#ifdef Q_OS_WIN
//    joy3D->read();
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

// this is doing in the OnbTurtle class
//#if defined(ONB)
//    device->update();
//#endif

    if (gamepad->isConnected())
    {
        joy->setPos(gamepad->axisLeftX(), -gamepad->axisLeftY());
        if (!turtle->isBusy())
        {
            if (gamepad->buttonL1() && !turtle->penState())
                turtle->penDown();
            else if (gamepad->buttonR1() && turtle->penState())
                turtle->penUp();

            if (gamepad->buttonA())
                turtle->setColor(0x00FF00);
            if (gamepad->buttonB())
                turtle->setColor(0xFF0000);
            if (gamepad->buttonX())
                turtle->setColor(0x0000FF);
            if (gamepad->buttonY())
                turtle->setColor(0xFFC000);

            if (gamepad->buttonStart())
                turtle->clearScreen();
        }

        if (gamepad->buttonLeft())
        {
            scene->setView(Scene::ViewMain);
            m_btnCamMain->setChecked(true);
        }
        else if (gamepad->buttonUp())
        {
            scene->setView(Scene::ViewTop);
            m_btnCamTop->setChecked(true);
        }
        else if (gamepad->buttonRight())
        {
            scene->setView(Scene::ViewFollow);
            m_btnCamFollow->setChecked(true);
        }
        else if (gamepad->buttonDown())
        {
            scene->setView(Scene::ViewChase);
            m_btnCamChase->setChecked(true);
        }

        QVector3D pos = scene->camera()->position();
        QVector3D dir = scene->camera()->direction();
        QVector3D top = scene->camera()->topDir();
        QVector3D side = QVector3D::crossProduct(dir, top);
        QVector3D dp;
        if (scene->view() == Scene::ViewTop)
            dp = side * gamepad->axisRightX() - top * gamepad->axisRightY();
        else
            dp = side * gamepad->axisRightX() - dir * gamepad->axisRightY();
        scene->camera()->setPosition(pos + dp * 0.3);
    }

    if (!btnPD->isChecked() && turtle->penState())
        btnPD->setChecked(true);
    else if (btnPD->isChecked() && !turtle->penState())
        btnPD->setChecked(false);

    if (!btnPU->isChecked() && !turtle->penState())
        btnPU->setChecked(true);
    else if (btnPU->isChecked() && turtle->penState())
        btnPU->setChecked(false);

    if (!isRunning())
    {
        float v = joy->y();
        float w = -joy->x();// * 10;
        if (turtle)
            turtle->setControl(v*3, w*3);
    }
    else
    {
        m_btnRun->setDown(true);
    }

    scene->integrate(dt);

#if defined(ONB)
    bool conn = device->isValid() && device->isPresent();
    connLed->setState(conn);
    if (conn)
        connlabel->setText("Черепашка подключилась!");
    else
        connlabel->setText("");
#endif

    scene->update();
    if (scene->isVisible())
        joy->update(); // THE HACK
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

//    m_btnRun->setEnabled(false);

    editor->clearHighlights();
    editor->setReadOnly(true);
    mProgramListView->setEnabled(false);
    QString text = editor->toPlainText();
//    if (!mProgramName.isEmpty() && mProgramName != "MAIN")
//        text = mProgramName;
//    setDebugMode(false);

    if (enableBtn->isVisible())
        enableBtn->setChecked(true);
    else
        showScene();

    logo->execute(text, mProgramName, mDebug);

    qDebug() << "*** RUN" << mProgramName << "***";
}


void MainWindow::step()
{
    if (!isRunning())
    {
        setDebugMode(true);
        run();
    }
    else
    {
//        m_btnRun->setEnabled(true);
        logo->setDebugMode(true);
        logo->doDebugStep();
    }
}

void MainWindow::stop()
{
    disconnect(logo, &QThread::finished, this, &MainWindow::stop);
    logo->stop();
    logo->wait(1000);
    if (logo->isRunning())
    {
        qDebug() << "EXTERMINATE!!";
        logo->terminate(); // hard stop
    }
    connect(logo, &QThread::finished, this, &MainWindow::stop);

    mProcessing = false;

    turtle->stop();

    if (!logo->isErrorState())
    {
        setDebugMode(false);
        console->setText(logo->result());
        console->selectAll();
        editor->clearHighlights();
    }
    else if (!mProgramName.isEmpty() && mProgramName != "#console")
    {
        setDebugMode(true);
    }

    console->setReadOnly(false);
    editor->setReadOnly(false);

    m_btnRun->setDown(false);
//    m_btnRun->setEnabled(true);

#if defined(ONB)
    device->stop();
    enableBtn->setChecked(false);
#endif

//    editor->setReadOnly(false);
    mProgramListView->setEnabled(true);
    qDebug() << "*** STOP ***";
}

void MainWindow::onLogoProcedureFetched(int start, int end)
{
    QString pname = logo->programName();
    if (pname == "#console")
        return;

    if (!editor->isVisible())
        return;

    if (mProgramName != pname && mPrograms.contains(pname))
    {
        open(pname);
    }

    QColor selColor(139, 237, 139);
    QColor lineColor(139, 227, 237, 192);
    editor->clearHighlights();
    editor->highlightText(start, end, selColor, lineColor);
    QTextCursor cur = editor->textCursor();
    cur.setPosition(start);
    editor->setTextCursor(cur);
    editor->ensureCursorVisible();
}

void MainWindow::onLogoError(QString programName, int start, int end, QString reason)
{
    if (mPrograms.contains(programName))
    {
        open(programName);
        editor->show();
    }

    if (programName == "#console")
    {
        console->setSelection(start, end);
    }
    else
    {
        editor->clearHighlights();
        editor->highlightText(start, end, Qt::red, QColor(255, 0, 0, 64));

        QTimer::singleShot(100, [=]()
        {
            QTextCursor cur = editor->textCursor();
            cur.setPosition(start);
            editor->setTextCursor(cur);
            editor->centerCursor();

        });
    }
}

void MainWindow::listPrograms()
{
    QDir dir;
    if (!dir.exists("programs"))
        dir.mkdir("programs");
    dir.cd("programs");
//    QFile mainFile("programs/main.txt");
//    if (!mainFile.exists())
//    {
//        mainFile.open(QIODevice::WriteOnly);
//        mainFile.write("");
//        mainFile.close();
//    }
    QStringList filters;
    filters << "*.txt";
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files, QDir::Name);

    mPrograms.clear();

    for (QFileInfo &file: files)
    {
        QString programName = file.baseName();//.toUpper();
        mPrograms << programName;
    }

    mProgramListModel->setStringList(mPrograms);

    open(mProgramName);
}

QString MainWindow::path(QString name) const
{
    return "programs/" + name + ".txt";
}

void MainWindow::save()
{
    QString text = editor->toPlainText();

    if (mProgramName.isEmpty())
    {
        qDebug() << "save WUT??";
        return;
    }

    logo->extractProcedures(text, mProgramName);
    mCommandListModel->setStringList(logo->procedures());
    mCommandListView->update();

//    QString firstLine = text.split("\n").first().trimmed();
//    QRegExp rx("^ЭТО\\s+(\\w+)", Qt::CaseInsensitive);
//    if (firstLine.indexOf(rx) != -1)
//    {
//        mProgramName = rx.cap(1);
//    }

    QString filename = path(mProgramName);
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
    QString filename = path(name);
    if (!QFile::exists(filename))
    {
        editor->hide();
        return;
    }
    mProgramName = name;
    QString text = load(name);
    editor->setPlainText(text);
    mProgramListView->setCurrentIndex(mProgramListModel->index(mPrograms.indexOf(name)));
    editor->show();
    editor->setFocus();
}

QString MainWindow::load(QString name)
{
    QString filename = path(name);
    QFile file(filename);
    if (!file.exists())
        return QString();
    file.open(QIODevice::ReadOnly);
    QString text = QString::fromUtf8(file.readAll());
    file.close();
    return text;
}

void MainWindow::updateCommands()
{
    for (const QString &name: mPrograms)
    {
        QString text = load(name);
        logo->extractProcedures(text, name);
    }

    mCommandListModel->setStringList(logo->procedures());
}

void MainWindow::setDebugMode(bool enabled)
{
    mDebug = enabled;
    if (mDebug)
    {
        if (btnScene->isVisible())
            showScene();
        editor->show();
    }
    else
    {
        if (btnScene->isVisible())
            showScene();
    }
}

void MainWindow::showScene()
{
    sceneBox->show();
    btnScene->setChecked(true);
}



