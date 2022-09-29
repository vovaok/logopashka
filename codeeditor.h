#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QPlainTextEdit>

class CodeEditor : public QPlainTextEdit
{
    Q_OBJECT

public:
    explicit CodeEditor(QWidget *parent=nullptr);

    void highlightLine(int num, QColor lineColor = Qt::yellow);
    void highlightText(int pos1, int pos2, QColor color = Qt::green, QColor lineColor = Qt::yellow);

public slots:
    void highlightCurrentLine(QColor lineColor = Qt::white);
    void clearHighlights();

protected:
    void keyPressEvent(QKeyEvent *e);

private:
    QList<QTextEdit::ExtraSelection> m_extraSelections;
};

#endif // CODEEDITOR_H
