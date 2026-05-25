#ifndef LEFTPANEL_H
#define LEFTPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>
#include <QMenu>
#include <QAction>
#include <QElapsedTimer>
#include "OcctWidget.h"


// ✅ ADD THIS
#include "ClientBackend.h"

class LeftPanel : public QWidget
{
    Q_OBJECT
public:
    // ✅ ADD backend to constructor
    OcctWidget* getMainOcctWidget() const { return myMainWidget; }
    explicit LeftPanel(ClientBackend *backend, QWidget *parent = nullptr);
    void toggleFooterSwipe();
    void setSwipeEnabled(bool enabled);
signals:
    void partSelectionStateChanged(bool isSelected);
    void requestTabChange(int tabIndex); // ✅ Add this new signal
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    // ✅ ADD THESE SLOTS to listen to the server
    void updateTelemetryUI();
    void updateDirectoryUI();

private:
    void setupUI();
    bool isSwipeUnlocked = false;
    QElapsedTimer m_uiThrottleTimer;
    QPoint dragStartPos;
    OcctWidget *myMainWidget;

    // ✅ ADD Backend Pointer
    ClientBackend *m_backend;


    // ✅ PROMOTE UI elements to class members so slots can update them
    QLabel *lblXYZ;
    QLabel *lblABC;
    QLabel *m_lblJoints[6];
    QVBoxLayout *m_mainLayout = nullptr;
    QStackedWidget *footerStack;

    QPushButton *m_btnServo;
    QPushButton *m_btnRun;
    QPushButton *m_btnStart;
    QPushButton *m_btnMode;
    QPushButton *m_btnSysHealth;

    QLabel *m_lblTP;
    QLabel *m_lblPR;
    QLabel *m_lblTR;
};

#endif // LEFTPANEL_H