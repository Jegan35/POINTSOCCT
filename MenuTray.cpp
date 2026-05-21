#include "MenuTray.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QFrame>
#include <QButtonGroup>

MenuTray::MenuTray(QWidget *parent) : QWidget(parent)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet("MenuTray { background-color: #1E1E24; border-left: 2px solid #3E3E42; }");

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setAlignment(Qt::AlignTop); // ✅ FIX: Forces everything to the top!
    layout->setContentsMargins(20, 25, 20, 25);
    layout->setSpacing(8);

    QLabel *lblHeader = new QLabel("≡  MENUS", this);
    lblHeader->setStyleSheet("color: #FFFFFF; font-weight: bold; font-size: 15px; border: none; padding-left: 5px;");
    layout->addWidget(lblHeader);

    QFrame *hLine = new QFrame(this);
    hLine->setFrameShape(QFrame::HLine);
    hLine->setStyleSheet("background-color: #3E3E42; max-height: 1px; border: none; margin-bottom: 10px;");
    layout->addWidget(hLine);

    QString baseBtn = "QPushButton { color: white; font-weight: bold; font-size: 15px; padding: 12px; border-radius: 6px; border: 2px solid transparent; } "
                      "QPushButton:checked { border: 2px solid white; } ";

    QString subBtnStyle = "QPushButton { background-color: #5B7282; color: white; font-weight: bold; padding: 10px; border-radius: 4px; } "
                          "QPushButton:checked { background-color: #0EA5E9; }";

    QPushButton *btnSpeed = new QPushButton("SPEED", this); btnSpeed->setStyleSheet(baseBtn + "QPushButton { background-color: #475569; }");
    layout->addWidget(btnSpeed);

    QPushButton *btnJog = new QPushButton("JOG ▾", this); btnJog->setStyleSheet(baseBtn + "QPushButton { background-color: #0284C7; }");
    layout->addWidget(btnJog);

    jogSubMenu = new QWidget(this);
    QHBoxLayout *jogSubLayout = new QHBoxLayout(jogSubMenu);
    jogSubLayout->setContentsMargins(10, 0, 10, 5);
    QPushButton *btnJogCart = new QPushButton("CARTESIAN", jogSubMenu); btnJogCart->setCheckable(true); btnJogCart->setStyleSheet(subBtnStyle);
    QPushButton *btnJogJnt = new QPushButton("JOINTS", jogSubMenu); btnJogJnt->setCheckable(true); btnJogJnt->setStyleSheet(subBtnStyle);
    jogSubLayout->addWidget(btnJogCart); jogSubLayout->addWidget(btnJogJnt);
    jogSubMenu->setVisible(false);
    layout->addWidget(jogSubMenu);

    QPushButton *btnMove = new QPushButton("MOVE ▾", this); btnMove->setStyleSheet(baseBtn + "QPushButton { background-color: #16A34A; }");
    layout->addWidget(btnMove);

    moveSubMenu = new QWidget(this);
    QHBoxLayout *moveSubLayout = new QHBoxLayout(moveSubMenu);
    moveSubLayout->setContentsMargins(10, 0, 10, 5);
    QPushButton *btnMoveCart = new QPushButton("CARTESIAN", moveSubMenu); btnMoveCart->setCheckable(true); btnMoveCart->setStyleSheet(subBtnStyle);
    QPushButton *btnMoveJnt = new QPushButton("JOINTS", moveSubMenu); btnMoveJnt->setCheckable(true); btnMoveJnt->setStyleSheet(subBtnStyle);
    moveSubLayout->addWidget(btnMoveCart); moveSubLayout->addWidget(btnMoveJnt);
    moveSubMenu->setVisible(false);
    layout->addWidget(moveSubMenu);

    QPushButton *btnManual = new QPushButton("MANUAL", this); btnManual->setStyleSheet(baseBtn + "QPushButton { background-color: #4F46E5; }");
    layout->addWidget(btnManual);

    QPushButton *btnRemote = new QPushButton("REMOTE", this); btnRemote->setStyleSheet(baseBtn + "QPushButton { background-color: #0F766E; }");
    layout->addWidget(btnRemote);

    QPushButton *btnAuto = new QPushButton("AUTO", this); btnAuto->setStyleSheet(baseBtn + "QPushButton { background-color: #7C3AED; }");
    layout->addWidget(btnAuto);

    layout->addStretch(); // Pushes the close button to the absolute bottom

    QPushButton *btnClose = new QPushButton("CLOSE", this); btnClose->setStyleSheet(baseBtn + "QPushButton { background-color: #DC2626; }");
    layout->addWidget(btnClose);

    slideAnimation = new QPropertyAnimation(this, "geometry");
    slideAnimation->setDuration(200);
    slideAnimation->setEasingCurve(QEasingCurve::OutCubic);

    // Routing
    connect(btnJog, &QPushButton::clicked, [this]() { jogSubMenu->setVisible(!jogSubMenu->isVisible()); moveSubMenu->setVisible(false); });
    connect(btnMove, &QPushButton::clicked, [this]() { moveSubMenu->setVisible(!moveSubMenu->isVisible()); jogSubMenu->setVisible(false); });
    connect(btnClose, &QPushButton::clicked, [this]() { toggleTray(parentWidget()->width(), parentWidget()->height()); });

    connect(btnSpeed, &QPushButton::clicked, [this]() { emit modeSelected("SPEED"); });
    connect(btnAuto, &QPushButton::clicked, [this]() { emit modeSelected("AUTO"); });
    connect(btnManual, &QPushButton::clicked, [this]() { emit modeSelected("MANUAL"); });
    connect(btnRemote, &QPushButton::clicked, [this]() { emit modeSelected("REMOTE"); });

    connect(btnJogCart, &QPushButton::clicked, [this, btnJogJnt]() { btnJogJnt->setChecked(false); emit modeSelected("JOG_CART"); });
    connect(btnJogJnt, &QPushButton::clicked, [this, btnJogCart]() { btnJogCart->setChecked(false); emit modeSelected("JOG_JNT"); });
    connect(btnMoveCart, &QPushButton::clicked, [this, btnMoveJnt]() { btnMoveJnt->setChecked(false); emit modeSelected("MOVE_CART"); });
    connect(btnMoveJnt, &QPushButton::clicked, [this, btnMoveCart]() { btnMoveCart->setChecked(false); emit modeSelected("MOVE_JNT"); });
}

void MenuTray::toggleTray(int parentWidth, int parentHeight)
{
    int trayWidth = 280;
    if (!isOpen) {
        setGeometry(parentWidth, 0, trayWidth, parentHeight);
        show(); raise();
        slideAnimation->setStartValue(QRect(parentWidth, 0, trayWidth, parentHeight));
        slideAnimation->setEndValue(QRect(parentWidth - trayWidth, 0, trayWidth, parentHeight));
        slideAnimation->start();
        isOpen = true;
    } else {
        slideAnimation->setStartValue(QRect(parentWidth - trayWidth, 0, trayWidth, parentHeight));
        slideAnimation->setEndValue(QRect(parentWidth, 0, trayWidth, parentHeight));
        slideAnimation->start();
        isOpen = false;
    }
}