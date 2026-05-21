#ifndef RIGHTPANEL_H
#define RIGHTPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QPushButton>
#include <QLabel>
#include <QTabWidget>

class RightPanel : public QWidget
{
    Q_OBJECT
public:
    explicit RightPanel(QWidget *parent = nullptr);

signals:
    // Define signals here to talk to the Left Panel later
    void jogCommand(QString axis, double direction);

private:
    void setupUI();
};

#endif // RIGHTPANEL_H