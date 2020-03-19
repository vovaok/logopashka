#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      mScriptIndex(-1),
      mProcessing(false),
      mDebug(false),
      mSpacer(nullptr)
{
    ui->setupUi(this);
    setWindowTitle("Robot");
    showFullScreen();

    setCursor(QCursor(QPixmap::fromImage(QImage(":/arrow.png")), 1, 4));

    mLoopCount = 0;

    QFile styleFile(":/style.css");
    styleFile.open(QIODevice::ReadOnly);
    QString css = styleFile.readAll();
    css.replace("$color1", "139, 237, 139");
    css.replace("$color2", "139, 227, 237");
    styleFile.close();
    setStyleSheet(css);   


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

void MainWindow::onTimer()
{
    float dt = 0.016f;

    if (mScriptIndex != -1 && !mProcessing && !mDebug)
    {
        mProcessing = true;
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
    mScript = editor->toPlainText().split('\n');
    mDebug = false;
    if (!device->isEnabled())
        enableBtn->setChecked(true);
    execLine(0);
}

void MainWindow::step()
{
    if (!isRunning())
    {
        run();
        return;
    }

    mScriptIndex++;

    execLine(mScriptIndex);

    qDebug() << "line #" << mScriptIndex << "loopcount:" << mLoopCount << "start:" << mLoopStartLine << "end:" << mLoopEndLine;

    if (mLoopCount)
    {
        if (mScriptIndex == mLoopEndLine)
        {
            --mLoopCount;
            if (mLoopCount )
                mScriptIndex = mLoopStartLine - 1;
        }
    }
}

void MainWindow::stop()
{
    mProcessing = false;
    mScriptIndex = -1;
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

void MainWindow::execLine(int num)
{
    if (num >= mScript.size())
    {
        stop();
        return;
    }

    mScriptIndex = num;
    editor->highlightLine(num, QColor(139, 227, 237));
    mProcessing = true;

    QString line = mScript[num];
    bool ok = parseLine(line.trimmed());
    if (!ok)
    {
        stop();
        editor->highlightLine(num, Qt::red);
    }
}

bool MainWindow::parseLine(QString line)
{
    line.replace("ё", "е").replace("Ё", "Е");
//    qDebug() << "line:" << line;
    QRegExp rx;
//    mProcessing = false;

    rx = QRegExp("^[\\[\\(\\{](.*)$");
    if (line.indexOf(rx) != -1)
    {
        mLoopStartLine = mScriptIndex;
        mLoopEndLine = -1; // find it later
        line = rx.cap(1).trimmed();
    }
    rx = QRegExp("^(.*)\\s*[\\[\\(\\{]$");
    if (line.indexOf(rx) != -1)
    {
        mLoopStartLine = mScriptIndex + 1;
        mLoopEndLine = -1; // find it later
        line = rx.cap(1).trimmed();
    }
    rx = QRegExp("^(.*)\\s*[\\]\\)\\}]$");
    if (line.indexOf(rx) != -1)
    {
        mLoopEndLine = mScriptIndex;
        line = rx.cap(1).trimmed();
    }

    qDebug() << "line:" << line;

    if (line.isEmpty())
    {
        mProcessing = false;
        return true;
    }

    rx = QRegExp("^(\\w+):$", Qt::CaseInsensitive);
    if (line.indexOf(rx) != -1)
    {
        qDebug() << "proc name:" << rx.cap(1);
        mProcessing = false;
        return true;
    }
    rx = QRegExp("^(\\w+)\\s+([\\d,\\.]+)$", Qt::CaseInsensitive);
    if (line.indexOf(rx) != -1)
    {
        QString cmd = rx.cap(1).toUpper();
        bool ok;
        float value = rx.cap(2).replace(',', '.').toFloat(&ok);
        if (!ok)
            return false;
//        qDebug() << rx.cap(1) << value;
        if (cmd == "ПОВТОР")
        {
            mLoopCount = value;
            mLoopStartLine = mScriptIndex + 1;
            mLoopEndLine = -1;// mLoopStartLine;
            mProcessing = false;
        }
        else if (cmd == "ВПЕРЕД")
        {
            device->forward(value);
        }
        else if (cmd == "НАЗАД")
        {
            device->backward(value);
        }
        else if (cmd == "ВЛЕВО" || cmd == "НАЛЕВО")
        {
            device->left(value);
        }
        else if (cmd == "ВПРАВО" || cmd == "НАПРАВО")
        {
            device->right(value);
        }
        else
        {
            return false;
        }
        return true;
    }
    rx = QRegExp("^(\\w+)$", Qt::CaseInsensitive);
    if (line.indexOf(rx) != -1)
    {
        QString cmd = line.toUpper();
//        qDebug() << rx.cap(1);
        if (cmd == "СТОП")
        {
            device->stop();
            stop();
        }
        else if (cmd == "ПП" || cmd == "ПЕРОПОДНЯТЬ" || cmd == "ПОДНЯТЬПЕРО")
        {
            device->penUp();
        }
        else if (cmd == "ПО" || cmd == "ОП" || cmd == "ПЕРООПУСТИТЬ" || cmd == "ОПУСТИТЬПЕРО")
        {
            device->penDown();
        }
        else
        {
            return false;
        }
        return true;
    }
    rx = QRegExp("^(\\w+)\\s+(\\w+)$", Qt::CaseInsensitive);
    if (line.indexOf(rx) != -1)
    {
        QString cmd = rx.cap(1).toUpper();
        QString param = rx.cap(2).toUpper();
//        qDebug () << cmd << param;
        if (cmd == "ПЕРО")
        {
            if (param == "ПОДНЯТЬ")
                device->penUp();
            else if (param == "ОПУСТИТЬ")
                device->penDown();
            else
                return false;
        }
        else if (cmd == "ПОДНЯТЬ")
        {
            if (param == "ПЕРО")
                device->penUp();
            else
                return false;
        }
        else if (cmd == "ОПУСТИТЬ")
        {
            if (param == "ПЕРО")
                device->penDown();
            else
                return false;
        }
        else
        {
            return false;
        }
        return true;
    }

    return false;
}
