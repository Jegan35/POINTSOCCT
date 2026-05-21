#ifndef RIGHTPANEL_H
#define RIGHTPANEL_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QStackedWidget>
#include <QTabWidget>
#include "RightHeader.h"
#include "MenuTray.h"

class RightPanel : public QWidget
{
    Q_OBJECT
public:
    explicit RightPanel(QWidget *parent = nullptr);

protected:
    void resizeEvent(QResizeEvent *event) override;

signals:
    void requestFooterSwipe(); // Tells MainWindow to swap the Left Panel
    void swipeLockToggled(bool isUnlocked);


private:
    void setupUI();
    QWidget* buildSpeedPanel();
    QWidget* buildJointsPanel();

    // ✅ NEW: Helper function to build clean tab screens
    QWidget* createPlaceholderTab(const QString &title);

    RightHeader *header;
    MenuTray *tray;
    QStackedWidget *controlStack;

    // State Tracking
    QString currentMovementMode = "JOG";

    // Cartesian D-Pad Elements
    QPushButton *btnToggleXYZ;
    QPushButton *btnToggleOrient;
    QPushButton *btnXPlus;
    QPushButton *btnXMinus;
    QPushButton *btnYPlus;
    QPushButton *btnYMinus;
    QPushButton *btnZPlus;
    QPushButton *btnZMinus;
};

#endif // RIGHTPANEL_H