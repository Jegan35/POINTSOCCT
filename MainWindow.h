#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSplitter>
#include <QTextEdit>
#include "OcctWidget.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void loadModel(const std::string& filepath);

private slots:
    void triggerOriginMode();
    void openLoadDialog();
    void resetOrigin();

    // NEW: Catches the click from the main view to update the side panel
    void onPartSelected(const TopoDS_Shape& shape, const QString& xyzData);

private:
    OcctWidget *myMainWidget;    // 70% Left View
    OcctWidget *mySideWidget;    // 30% Top-Right View
    QTextEdit  *myCoordinateLog; // 30% Bottom-Right Text
};

#endif // MAINWINDOW_H