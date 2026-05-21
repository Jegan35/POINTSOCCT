#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include "LeftPanel.h"
#include "RightPanel.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    QSplitter *mainSplitter;
    LeftPanel *leftPanel;
    RightPanel *rightPanel;
};

#endif // MAINWINDOW_H