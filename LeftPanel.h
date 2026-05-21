#ifndef LEFTPANEL_H
#define LEFTPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include "OcctWidget.h"

class LeftPanel : public QWidget
{
    Q_OBJECT
public:
    explicit LeftPanel(QWidget *parent = nullptr);

private:
    void setupUI();

    OcctWidget *myMainWidget;
    // UI Elements for later updating
    QLabel *lblXYZ;
    QLabel *lblABC;
};

#endif // LEFTPANEL_H