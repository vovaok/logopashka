#ifndef CONSOLEEDIT_H
#define CONSOLEEDIT_H

#include <QLineEdit>
#include <QKeyEvent>

class ConsoleEdit : public QLineEdit
{
    Q_OBJECT
public:
    ConsoleEdit(QWidget *parent = 0);

protected:
    void keyPressEvent(QKeyEvent *event) override;

private:
    QStringList m_history;
    int m_historyKey;

    void onReturnPressed();
};

#endif // CONSOLEEDIT_H
