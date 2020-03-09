#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow),
      mScriptIndex(-1),
      mProcessing(false),
      mDebug(false)
{
    ui->setupUi(this);
    setWindowTitle("Robot");
//    showFullScreen();

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

    connlabel = new QLabel("disconnected");
    ui->statusbar->addWidget(connlabel);

    QPushButton *stopBtn = new QPushButton("STOP");
    connect(stopBtn, &QPushButton::clicked, this, &MainWindow::stop);

    enableBtn = new QPushButton("Enable");
    enableBtn->setCheckable(true);
    connect(enableBtn, &QPushButton::clicked, [=](bool checked)
    {
        device->setEnabled(checked);
    });

    joy = new JoystickWidget;
    joy->setColor(QColor(127, 185, 187));
//    joy->setRadius(0.1, 0.23);
    joy->setMinimumSize(300, 300);
    joy->resize(600, 600);

//    QGroupBox *joyBox = new QGroupBox;
//    QVBoxLayout *jlay = new QVBoxLayout;
//    joyBox->setLayout(jlay);

    editor = new CodeEditor;


    QPushButton *btnPlay = new QPushButton("RUN");
    connect(btnPlay, &QPushButton::clicked, this, &MainWindow::run);

//    QPushButton *btnSave = new QPushButton("Save");
//    connect(btnSave, &QPushButton::clicked, [=]()
//    {
//        QString fname = QFileDialog::getSaveFileName(0L, "", "", "*.csv");
//        if (!fname.isEmpty())
//        {
//            QFile f(fname);
//            f.open(QIODevice::WriteOnly);
//            for (int j=0; j<mScriptTable->rowCount(); j++)
//            {
//                for (int i=0; i<8; i++)
//                {
//                    f.write(mScriptTable->item(j, i)->text().toUtf8());
//                    f.write(";");
//                }
//                f.write("\n");
//            }
//            f.close();
//        }
//    });

//    QPushButton *btnLoad = new QPushButton("Load");
//    connect(btnLoad, &QPushButton::clicked, [=]()
//    {
//        QString fname = QFileDialog::getOpenFileName(0L, "", "", "*.csv");
//        if (!fname.isEmpty())
//        {
//            mScriptTable->clearContents();
//            QFile f(fname);
//            f.open(QIODevice::ReadOnly);
//            QString line;
//            do
//            {
//                line = f.readLine().trimmed();
//                if (line.isEmpty())
//                    break;
//                QStringList s = line.split(";");
//                int idx = mScriptTable->rowCount();
//                mScriptTable->insertRow(idx);
//                for (int i=0; i<8; i++)
//                {
//                    if (i >= s.size())
//                        break;
//                    mScriptTable->setItem(idx, i, new QTableWidgetItem(s[i].trimmed()));
//                }
//            }
//            while (!line.isEmpty());
//            f.close();
//        }
//    });


    QGroupBox *scriptBox = new QGroupBox();
    QGridLayout *scriptLay = new QGridLayout;
    scriptBox->setLayout(scriptLay);
    scriptLay->addWidget(editor, 0, 0, 4, 1);
    scriptLay->addWidget(btnPlay, 0, 1);
    scriptLay->addWidget(stopBtn, 1, 1);
    scriptLay->addWidget(joy, 2, 1);
    scriptLay->addWidget(enableBtn, 3, 1);
//    scriptLay->addWidget(btnLoad, 0, 3);
//    scriptLay->addWidget(btnSave, 0, 4);
//    scriptLay->addWidget(mScriptTable, 1, 0, 1, 5);

    stack = new QStackedWidget;
//    stack->addWidget(ctrlBox);
    stack->addWidget(scriptBox);

//    QPushButton *scriptBtn = new QPushButton("script");
//    connect(scriptBtn, &QPushButton::clicked, [=]()
//    {
//        stack->setCurrentIndex(1 - stack->currentIndex());
//    });

    QHBoxLayout *lay = new QHBoxLayout;
//    lay->addWidget(scriptBtn);
    lay->addWidget(stack, 1);
//    lay->addWidget(joyBox, 0);

    ui->centralwidget->setLayout(lay);

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

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    if ((e->modifiers() & Qt::ControlModifier) && e->key() == Qt::Key_R)
        run();
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

    if (device->isValid())
        connlabel->setText("connected");
    else
        connlabel->setText("disconnected");
}

void MainWindow::run()
{
    editor->setReadOnly(true);
    mScript = editor->toPlainText().split('\n');
    mDebug = false;
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
    editor->setReadOnly(false);
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
    mProcessing = false;

    rx = QRegExp("^[\\[\\(\\{)](.*)$");
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
    rx = QRegExp("^(\\w+)\\s+(\\d+)$", Qt::CaseInsensitive);
    if (line.indexOf(rx) != -1)
    {
        QString cmd = rx.cap(1).toUpper();
        int value = rx.cap(2).toInt();
//        qDebug() << rx.cap(1) << value;
        if (cmd == "ПОВТОР")
        {
            mLoopCount = value;
            mLoopStartLine = mScriptIndex + 1;
            mLoopEndLine = mLoopStartLine;
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
        else if (cmd == "ВЛЕВО")
        {
            device->left(value);
        }
        else if (cmd == "ВПРАВО")
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
        else if (cmd == "ПО" || cmd == "ПЕРОПОДНЯТЬ")
        {
            mProcessing = false;
        }
        else if (cmd == "ПП" || cmd == "ПЕРООПУСТИТЬ")
        {
            mProcessing = false;
        }
        else
        {
            return false;
        }
        return true;
    }

    return false;
}
