#ifndef LEFTPANEL_H
#define LEFTPANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget> // ✅ ADD THIS LINE
#include <QMenu>          // ✅ Ensure this is also here for your menu
#include <QAction>        // ✅ Ensure this is also here
#include "OcctWidget.h"

class LeftPanel : public QWidget
{
    Q_OBJECT
public:
    explicit LeftPanel(QWidget *parent = nullptr);
    void toggleFooterSwipe(); // Make sure this is declared
    void setSwipeEnabled(bool enabled);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override; // ✅ Intercepts the mouse

private:
    void setupUI();
    bool isSwipeUnlocked = false;
    QPoint dragStartPos;
    OcctWidget *myMainWidget;
    QLabel *lblXYZ;
    QLabel *lblABC;
    QStackedWidget *footerStack; // This will now work
};

#endif // LEFTPANEL_H