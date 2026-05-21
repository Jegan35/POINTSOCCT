#include "RightPanel.h"

RightPanel::RightPanel(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void RightPanel::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(20);

    // --- TOP: Status Header ---
    QHBoxLayout *headerLayout = new QHBoxLayout();
    QLabel *lblMenu = new QLabel("≡ MENU", this);
    lblMenu->setStyleSheet("color: white; font-size: 20px; font-weight: bold;");

    QLabel *lblStatus = new QLabel("MANUAL | JOG : CARTESIAN", this);
    lblStatus->setStyleSheet("color: #10B981; font-weight: bold; font-size: 14px;");

    QPushButton *btnStandby = new QPushButton("STANDBY", this);
    btnStandby->setStyleSheet("background-color: transparent; border: 2px solid #10B981; color: #10B981; font-weight: bold; padding: 5px 15px;");

    headerLayout->addWidget(lblMenu);
    headerLayout->addStretch();
    headerLayout->addWidget(lblStatus);
    headerLayout->addStretch();
    headerLayout->addWidget(btnStandby);
    mainLayout->addLayout(headerLayout);

    // --- MIDDLE: Cartesian Jog Pad ---
    QGridLayout *jogGrid = new QGridLayout();
    QString padStyle = "background-color: #3B82F6; color: white; font-weight: bold; font-size: 18px; border-radius: 5px; min-width: 60px; min-height: 60px;";
    QString padStyleRed = "background-color: #EF4444; color: white; font-weight: bold; font-size: 18px; border-radius: 5px; min-width: 60px; min-height: 60px;";
    QString padStyleGreen = "background-color: #10B981; color: white; font-weight: bold; font-size: 18px; border-radius: 5px; min-width: 60px; min-height: 60px;";

    QPushButton *btnYPlus = new QPushButton("Y+", this); btnYPlus->setStyleSheet(padStyleGreen);
    QPushButton *btnYMinus = new QPushButton("Y-", this); btnYMinus->setStyleSheet(padStyleGreen);
    QPushButton *btnXPlus = new QPushButton("X+", this); btnXPlus->setStyleSheet(padStyleRed);
    QPushButton *btnXMinus = new QPushButton("X-", this); btnXMinus->setStyleSheet(padStyleRed);
    QPushButton *btnZPlus = new QPushButton("Z+", this); btnZPlus->setStyleSheet(padStyle);
    QPushButton *btnZMinus = new QPushButton("Z-", this); btnZMinus->setStyleSheet(padStyle);

    QPushButton *btnHomeJog = new QPushButton("HOME", this);
    btnHomeJog->setStyleSheet("background-color: #007ACC; color: white; font-weight: bold; border-radius: 30px; min-width: 60px; min-height: 60px;"); // Circle

    // Layout the cross-pad
    jogGrid->addWidget(btnYPlus, 0, 1);
    jogGrid->addWidget(btnXMinus, 1, 0);
    jogGrid->addWidget(btnHomeJog, 1, 1);
    jogGrid->addWidget(btnXPlus, 1, 2);
    jogGrid->addWidget(btnYMinus, 2, 1);

    // Z axis controls offset to the right
    jogGrid->addWidget(btnZPlus, 0, 3);
    jogGrid->addWidget(btnZMinus, 1, 3);

    mainLayout->addLayout(jogGrid);
    mainLayout->addStretch();

    // --- BOTTOM: Workspace Area ---
    QTabWidget *workspaceTabs = new QTabWidget(this);
    workspaceTabs->setStyleSheet("QTabWidget::pane { border: 1px solid #3E3E42; background: white; border-radius: 5px;}");

    QWidget *tabFile = new QWidget();
    QVBoxLayout *tabLayout = new QVBoxLayout(tabFile);
    QLabel *lblPlaceholder = new QLabel("NO ACTIVE TABLES DISPLAYED\nClick the circular toggles in the top toolbar to view TP Points.", this);
    lblPlaceholder->setAlignment(Qt::AlignCenter);
    lblPlaceholder->setStyleSheet("color: gray; font-weight: bold;");
    tabLayout->addWidget(lblPlaceholder);

    workspaceTabs->addTab(tabFile, "Programs File");
    workspaceTabs->setMinimumHeight(250);

    mainLayout->addWidget(workspaceTabs);
}