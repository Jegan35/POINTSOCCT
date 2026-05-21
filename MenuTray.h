#ifndef MENUTRAY_H
#define MENUTRAY_H

#include <QWidget>
#include <QPropertyAnimation>

class MenuTray : public QWidget
{
    Q_OBJECT
public:
    explicit MenuTray(QWidget *parent = nullptr);
    void toggleTray(int parentWidth, int parentHeight);

signals:
    void modeSelected(const QString &mode); // The signal to tell RightPanel what to do

private:
    bool isOpen = false;
    QPropertyAnimation *slideAnimation;

    // Sub-menus for Jog and Move
    QWidget *jogSubMenu;
    QWidget *moveSubMenu;
};

#endif // MENUTRAY_H