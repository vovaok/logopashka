#include "consoleedit.h"

ConsoleEdit::ConsoleEdit(QWidget *parent) :
    QLineEdit(parent),
    m_historyKey(0)
{
    setPlaceholderText("? ");
    m_history.append("");
    connect(this, &QLineEdit::returnPressed, this, &ConsoleEdit::onReturnPressed);
}

void ConsoleEdit::keyPressEvent(QKeyEvent *event)
{
    if (m_history.isEmpty())
        return;

    if (event->key() == Qt::Key_Up || event->key() == Qt::Key_Down)
    {
        if (m_historyKey == m_history.count() - 1)
            m_history.last() = text();

        if (event->key() == Qt::Key_Up)
            m_historyKey--;
        else
            m_historyKey++;

        m_historyKey = qBound(0, m_historyKey, m_history.count() - 1);

        setText(m_history[m_historyKey]);
        event->accept();
    }
    else
    {
        QLineEdit::keyPressEvent(event);
    }
}

void ConsoleEdit::onReturnPressed()
{
//    if(m_completer->popup()->isVisible()) return;
    m_history.last() = text();
    m_history.append("");
    m_historyKey = m_history.count() - 1;
}
