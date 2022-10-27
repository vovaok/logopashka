#include "codeeditor.h"
#include <QScrollBar>
#include <QDebug>

CodeEditor::CodeEditor(QWidget *parent) :
    QPlainTextEdit(parent)
{
    QFontMetrics fm(font());
    int em = fm.width('m');
    setViewportMargins(1*em, .5*em, 0, .5*em);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    connect(verticalScrollBar(), &QScrollBar::rangeChanged, [=](int min, int max)
    {
        verticalScrollBar()->setHidden(min == max);
    });

    connect(this, &CodeEditor::textChanged, this, &CodeEditor::clearHighlights);
}

void CodeEditor::highlightLine(int num, QColor lineColor)
{
    if (num >= 0)
    {
        QTextEdit::ExtraSelection selection;

        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        QTextCursor cur = textCursor();
        cur.movePosition(QTextCursor::Start);
        cur.movePosition(QTextCursor::Down, QTextCursor::MoveAnchor, num);
        selection.cursor = cur;
        selection.cursor.clearSelection();
        m_extraSelections.append(selection);
    }

    setExtraSelections(m_extraSelections);
}

void CodeEditor::highlightText(int pos1, int pos2, QColor color, QColor lineColor)
{
    if (pos1 >= 0)
    {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(lineColor);
//        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        QTextCursor cur = textCursor();
        cur.setPosition(pos1);
        selection.cursor = cur;
        selection.cursor.clearSelection();
        selection.cursor.setPosition(pos2, QTextCursor::KeepAnchor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        m_extraSelections.append(selection);

        selection.format.setBackground(color);
        selection.format.setTextOutline(QPen(Qt::black)); // bold
        selection.format.setProperty(QTextFormat::FullWidthSelection, false);
//        selection.cursor.setPosition(pos2, QTextCursor::KeepAnchor);
        m_extraSelections.append(selection);
    }

    setExtraSelections(m_extraSelections);
}

void CodeEditor::highlightCurrentLine(QColor lineColor)
{
    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;
        selection.format.setBackground(lineColor);
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        m_extraSelections.append(selection);
    }

    setExtraSelections(m_extraSelections);
}

void CodeEditor::clearHighlights()
{
    m_extraSelections.clear();
    setExtraSelections(m_extraSelections);
}

void CodeEditor::keyPressEvent(QKeyEvent *e)
{
    if (e->text().toLower() == "х" && e->modifiers() & Qt::ControlModifier)
    {
        insertPlainText(e->modifiers() & Qt::ShiftModifier? "{": "[");
        e->accept();
    }
    else if (e->text().toLower() == "ъ" && e->modifiers() & Qt::ControlModifier)
    {
        insertPlainText(e->modifiers() & Qt::ShiftModifier? "}": "]");
        e->accept();
    }
    else
    {
        QPlainTextEdit::keyPressEvent(e);
    }
}
