#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent=nullptr);

    void highlightLine(int num, QColor lineColor = Qt::yellow);

public slots:
    void highlightCurrentLine();

protected:

private:

};

#endif // CODEEDITOR_H
