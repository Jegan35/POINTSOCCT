#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include "LeftPanel.h"
#include "RightPanel.h"
#include "MenuTray.h"
#include "ClientBackend.h" // ✅ MUST INCLUDE THIS

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    LeftPanel *leftPanel;
    RightPanel *rightPanel;
    QSplitter *mainSplitter;
    MenuTray *menuTray;

    ClientBackend *m_backend; // ✅ MUST ADD THIS
};

#endif // MAINWINDOW_H