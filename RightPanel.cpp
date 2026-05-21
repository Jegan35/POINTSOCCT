#include "RightPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QResizeEvent>
#include <QFrame>
#include <QFormLayout>
#include <QComboBox>
#include <QDoubleSpinBox>
#include <QSlider>

RightPanel::RightPanel(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void RightPanel::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    // ==========================================
    // 1. TOP 10%: Header
    // ==========================================
    header = new RightHeader(this);
    header->setStyleSheet("background-color: #1E1E24; border-bottom: 2px solid #3E3E42;");

    // ==========================================
    // 2. MIDDLE 40%: The Stacked Control Area
    // ==========================================
    controlStack = new QStackedWidget(this);

    // ✅ FIX: Force the stack to obey layout constraints
    controlStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    // --- Card 0: Cartesian D-Pad ---
    QWidget *cartesianContainer = new QWidget(this);
    QVBoxLayout *dpadLayout = new QVBoxLayout(cartesianContainer);
    dpadLayout->setContentsMargins(20, 5, 20, 5); // Reduced margins to save space
    dpadLayout->setSpacing(10);

    // Toggle Pill (XYZ / RxRyRz)
    QHBoxLayout *toggleLayout = new QHBoxLayout();
    QFrame *toggleFrame = new QFrame(this);
    toggleFrame->setStyleSheet("QFrame { background-color: #1E1E24; border: 1px solid #3E3E42; border-radius: 18px; }");
    toggleFrame->setFixedSize(180, 36); // Slightly smaller

    QHBoxLayout *pillLayout = new QHBoxLayout(toggleFrame);
    pillLayout->setContentsMargins(2, 2, 2, 2);
    pillLayout->setSpacing(0);

    btnToggleXYZ = new QPushButton("CART", toggleFrame);
    btnToggleOrient = new QPushButton("JNT", toggleFrame);

    QString toggleBase = "QPushButton { color: white; font-weight: bold; border: none; border-radius: 14px; }";
    QString xyzActive = toggleBase + "QPushButton { background-color: #00E5FF; color: black; }";
    QString orientActive = toggleBase + "QPushButton { background-color: #22C55E; color: white; }";
    QString inactive = toggleBase + "QPushButton { background-color: transparent; }";

    btnToggleXYZ->setStyleSheet(xyzActive);
    btnToggleOrient->setStyleSheet(inactive);

    pillLayout->addWidget(btnToggleXYZ);
    pillLayout->addWidget(btnToggleOrient);
    toggleLayout->addWidget(toggleFrame);
    toggleLayout->addStretch();
    dpadLayout->addLayout(toggleLayout);

    // The 3D Grid Pad
    QGridLayout *jogGrid = new QGridLayout();
    jogGrid->setSpacing(10);
    jogGrid->setAlignment(Qt::AlignCenter);

    // ✅ FIX: Reduced sizes from 80px to 65px so it fits STRICTLY inside the 40% area on all screens!
    QString btnRed = "QPushButton { background-color: #EF4444; color: white; font-weight: bold; font-size: 22px; border-radius: 8px; border-bottom: 5px solid #B91C1C; min-width: 65px; min-height: 65px; } QPushButton:pressed { margin-top: 5px; border-bottom: 0px; }";
    QString btnGreen = "QPushButton { background-color: #4ADE80; color: white; font-weight: bold; font-size: 22px; border-radius: 8px; border-bottom: 5px solid #16A34A; min-width: 65px; min-height: 65px; } QPushButton:pressed { margin-top: 5px; border-bottom: 0px; }";
    QString btnBlue = "QPushButton { background-color: #0EA5E9; color: white; font-weight: bold; font-size: 22px; border-radius: 8px; border-bottom: 5px solid #0369A1; min-width: 65px; min-height: 65px; } QPushButton:pressed { margin-top: 5px; border-bottom: 0px; }";
    QString btnHomeCirc = "QPushButton { background-color: #0284C7; color: white; font-weight: bold; font-size: 14px; border-radius: 32px; border-bottom: 5px solid #075985; min-width: 65px; min-height: 65px; max-width: 65px; max-height: 65px; } QPushButton:pressed { margin-top: 5px; border-bottom: 0px; }";

    btnYPlus = new QPushButton("Y+", this); btnYPlus->setStyleSheet(btnGreen);
    btnYMinus = new QPushButton("Y-", this); btnYMinus->setStyleSheet(btnGreen);
    btnXPlus = new QPushButton("X+", this); btnXPlus->setStyleSheet(btnRed);
    btnXMinus = new QPushButton("X-", this); btnXMinus->setStyleSheet(btnRed);
    btnZPlus = new QPushButton("Z+", this); btnZPlus->setStyleSheet(btnBlue);
    btnZMinus = new QPushButton("Z-", this); btnZMinus->setStyleSheet(btnBlue);
    QPushButton *btnHomeJog = new QPushButton("HOME", this); btnHomeJog->setStyleSheet(btnHomeCirc);

    // Cross shape
    jogGrid->addWidget(btnYPlus, 0, 1);
    jogGrid->addWidget(btnXMinus, 1, 0);
    jogGrid->addWidget(btnHomeJog, 1, 1);
    jogGrid->addWidget(btnXPlus, 1, 2);
    jogGrid->addWidget(btnYMinus, 2, 1);

    // ✅ FIX: Placed Z+ and Z- strictly in their own row cells so they NEVER overlap and disappear.
    jogGrid->addWidget(btnZPlus, 0, 3);
    jogGrid->addWidget(btnZMinus, 2, 3);

    dpadLayout->addLayout(jogGrid);

    // --- Populate the Stack ---
    controlStack->addWidget(cartesianContainer);     // Index 0: Cartesian
    controlStack->addWidget(buildSpeedPanel());      // Index 1: Speed
    controlStack->addWidget(buildJointsPanel());     // Index 2: Joints

    // ==========================================
    // 3. BOTTOM 50%: Program/TP File Tabs
    // ==========================================
    QTabWidget *workspaceTabs = new QTabWidget(this);
    workspaceTabs->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    // ✅ NEW STYLING: Exactly matches your uploaded tab image!
    workspaceTabs->setStyleSheet(
        "QTabWidget::pane { border-top: 3px solid #00E5FF; background: #FFFFFF; } " // White background!
        "QTabBar::tab { background: #1E1E24; color: #9CA3AF; padding: 12px 20px; font-weight: bold; font-size: 13px; text-transform: uppercase; } "
        "QTabBar::tab:selected { color: #00E5FF; border-bottom: 3px solid #00E5FF; } "
        "QTabBar::tab:hover { color: #FFFFFF; }"
        );

    // Create the MAX button for the far right corner
    QPushButton *btnMax = new QPushButton(" MAX", this);
    btnMax->setStyleSheet("color: #00E5FF; background: transparent; font-weight: bold; font-size: 14px; border: none; padding-right: 20px;");
    btnMax->setCursor(Qt::PointingHandCursor);
    workspaceTabs->setCornerWidget(btnMax, Qt::TopRightCorner);

    // Add all 5 separate isolated tabs
    workspaceTabs->addTab(createPlaceholderTab("PLEASE OPEN A PROGRAM FILE"), "PROGRAM FILE");
    workspaceTabs->addTab(createPlaceholderTab("I/O PANEL SETUP AND STATUS"), "I/O PANEL");
    workspaceTabs->addTab(createPlaceholderTab("TP FILE DATA AND EXECUTION"), "TP FILE");
    workspaceTabs->addTab(createPlaceholderTab("SYSTEM DATA VARIABLES"), "DATA VAR");
    workspaceTabs->addTab(createPlaceholderTab("ROBOT AXIS LIMITS"), "AXIS LIMIT");

    // ==========================================
    // ASSEMBLE MAIN LAYOUT WITH STRICT RATIOS
    // ==========================================
    mainLayout->addWidget(header, 1);
    mainLayout->addWidget(controlStack, 4);
    mainLayout->addWidget(workspaceTabs, 5);

    // ==========================================
    // 4. WIRING & MENU TRAY
    // ==========================================
    tray = new MenuTray(this);
    tray->hide();

    connect(header, &RightHeader::menuClicked, [this]() {
        tray->toggleTray(this->width(), this->height());
    });

    connect(header, &RightHeader::swipeLockChanged, this, [this](bool isUnlocked) {
        emit swipeLockToggled(isUnlocked);
    });

    // Routing from the Menu Tray
    connect(tray, &MenuTray::modeSelected, [this](const QString &mode) {
        if (mode == "SPEED") {
            header->updateStatusText("SETTINGS | GLOBAL SPEED", false);
            controlStack->setCurrentIndex(1);
        }
        else if (mode == "JOG_CART") {
            currentMovementMode = "JOG";
            header->updateStatusText("MANUAL | JOG : CARTESIAN", false);
            controlStack->setCurrentIndex(0);
        }
        else if (mode == "JOG_JNT") {
            currentMovementMode = "JOG";
            header->updateStatusText("MANUAL | JOG : JOINTS", false);
            controlStack->setCurrentIndex(2);
        }
        else if (mode == "MOVE_CART") {
            currentMovementMode = "MOVE";
            header->updateStatusText("MANUAL | MOVE : CARTESIAN", false);
            controlStack->setCurrentIndex(0);
        }
        else if (mode == "MOVE_JNT") {
            currentMovementMode = "MOVE";
            header->updateStatusText("MANUAL | MOVE : JOINTS", false);
            controlStack->setCurrentIndex(2);
        }
        else if (mode == "AUTO") {
            header->updateStatusText("AUTO | RUNNING", true); // ✅ Sets Bulb to ORANGE
        }
        else if (mode == "MANUAL" || mode == "REMOTE") {
            header->updateStatusText(mode + " | STANDBY", false);
        }
        tray->toggleTray(this->width(), this->height());
    });

    // D-Pad Toggle Logic
    connect(btnToggleXYZ, &QPushButton::clicked, [this, xyzActive, inactive]() {
        btnToggleXYZ->setStyleSheet(xyzActive);
        btnToggleOrient->setStyleSheet(inactive);
        header->updateStatusText("MANUAL | " + currentMovementMode + " : CARTESIAN", false);
        btnXPlus->setText("X+"); btnXMinus->setText("X-");
        btnYPlus->setText("Y+"); btnYMinus->setText("Y-");
        btnZPlus->setText("Z+"); btnZMinus->setText("Z-");
    });

    connect(btnToggleOrient, &QPushButton::clicked, [this, orientActive, inactive]() {
        btnToggleOrient->setStyleSheet(orientActive);
        btnToggleXYZ->setStyleSheet(inactive);
        header->updateStatusText("MANUAL | " + currentMovementMode + " : JOINT (RxRyRz)", false);
        btnXPlus->setText("Rx+"); btnXMinus->setText("Rx-");
        btnYPlus->setText("Ry+"); btnYMinus->setText("Ry-");
        btnZPlus->setText("Rz+"); btnZMinus->setText("Rz-");
    });
    connect(header, &RightHeader::swipeLockChanged, this, [this](bool isUnlocked) {
        emit swipeLockToggled(isUnlocked);
    });
}

// ✅ NEW HELPER: Creates a blank white screen with grey text for the Tabs
QWidget* RightPanel::createPlaceholderTab(const QString &title)
{
    QWidget *tab = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(tab);
    QLabel *lbl = new QLabel(title, tab);
    lbl->setAlignment(Qt::AlignCenter);
    lbl->setStyleSheet("color: #9CA3AF; font-weight: bold; font-size: 18px; letter-spacing: 2px;");
    layout->addWidget(lbl);
    return tab;
}

void RightPanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (!tray->isHidden()) {
        int trayWidth = 280;
        tray->setGeometry(this->width() - trayWidth, 0, trayWidth, this->height());
    }
}

QWidget* RightPanel::buildJointsPanel()
{
    QWidget *panel = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(panel);
    mainLayout->setAlignment(Qt::AlignCenter);

    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(8);

    QLabel *lblNeg = new QLabel("NEG"); lblNeg->setStyleSheet("color: #EF4444; font-weight: bold; text-align: center;"); lblNeg->setAlignment(Qt::AlignCenter);
    QLabel *lblBase = new QLabel("BASE / WRIST"); lblBase->setStyleSheet("color: white; font-weight: bold; text-align: center;"); lblBase->setAlignment(Qt::AlignCenter);
    QLabel *lblPos = new QLabel("POS"); lblPos->setStyleSheet("color: #22C55E; font-weight: bold; text-align: center;"); lblPos->setAlignment(Qt::AlignCenter);

    grid->addWidget(lblNeg, 0, 0);
    grid->addWidget(lblBase, 0, 1);
    grid->addWidget(lblPos, 0, 2);

    QString btnRedStyle = "QPushButton { background-color: #EF4444; color: white; font-weight: bold; font-size: 20px; border-radius: 4px; border-bottom: 4px solid #B91C1C; min-width: 100px; min-height: 45px; } QPushButton:pressed { margin-top: 4px; border-bottom: 0px; }";
    QString btnGrnStyle = "QPushButton { background-color: #4ADE80; color: white; font-weight: bold; font-size: 20px; border-radius: 4px; border-bottom: 4px solid #16A34A; min-width: 100px; min-height: 45px; } QPushButton:pressed { margin-top: 4px; border-bottom: 0px; }";
    QString lblJointStyle = "QLabel { color: white; font-weight: bold; font-size: 16px; }";

    for (int i = 1; i <= 6; ++i) {
        QPushButton *btnNeg = new QPushButton(QString("J%1-").arg(i)); btnNeg->setStyleSheet(btnRedStyle);
        QLabel *lblJ = new QLabel(QString("J%1").arg(i)); lblJ->setStyleSheet(lblJointStyle); lblJ->setAlignment(Qt::AlignCenter);
        QPushButton *btnPos = new QPushButton(QString("J%1+").arg(i)); btnPos->setStyleSheet(btnGrnStyle);

        grid->addWidget(btnNeg, i, 0);
        grid->addWidget(lblJ, i, 1);
        grid->addWidget(btnPos, i, 2);
    }
    mainLayout->addLayout(grid);
    return panel;
}

// [KEEP YOUR buildSpeedPanel() EXACTLY AS IT WAS HERE]
QWidget* RightPanel::buildSpeedPanel()
{
    QWidget *panel = new QWidget(this);
    QHBoxLayout *mainLayout = new QHBoxLayout(panel);
    mainLayout->setContentsMargins(20, 10, 20, 10);

    QString comboStyle = "QComboBox { background-color: white; color: black; font-weight: bold; padding: 5px; border-radius: 2px; min-width: 100px; }";
    QString spinStyle = "QDoubleSpinBox { background-color: white; color: black; font-weight: bold; padding: 5px; border-radius: 2px; min-width: 100px; }";
    QString lblStyle = "QLabel { color: white; font-weight: bold; font-size: 13px; }";

    QWidget *formWidget = new QWidget();
    QFormLayout *form = new QFormLayout(formWidget);
    form->setLabelAlignment(Qt::AlignRight | Qt::AlignVCenter);
    form->setSpacing(15);

    QComboBox *cmbMmInc = new QComboBox(); cmbMmInc->addItems({"mm", "50", "25", "15", "10", "5", "2", "1", "0.1", "0.01", "0.001"}); cmbMmInc->setStyleSheet(comboStyle);
    QDoubleSpinBox *spnMms = new QDoubleSpinBox(); spnMms->setRange(0.1, 500.0); spnMms->setValue(50.0); spnMms->setStyleSheet(spinStyle);
    QComboBox *cmbDegInc = new QComboBox(); cmbDegInc->addItems({"deg", "20", "15", "10", "5", "2", "1", "0.1", "0.01", "0.0001"}); cmbDegInc->setStyleSheet(comboStyle);
    QDoubleSpinBox *spnDegs = new QDoubleSpinBox(); spnDegs->setRange(0.1, 500.0); spnDegs->setValue(50.0); spnDegs->setStyleSheet(spinStyle);
    QComboBox *cmbFrame = new QComboBox(); cmbFrame->addItems({"frames", "Base", "Tool", "User"}); cmbFrame->setStyleSheet(comboStyle);

    QLabel *l1 = new QLabel("MM Inc:"); l1->setStyleSheet(lblStyle); form->addRow(l1, cmbMmInc);
    QLabel *l2 = new QLabel("MM/S:"); l2->setStyleSheet(lblStyle); form->addRow(l2, spnMms);
    QLabel *l3 = new QLabel("DEG Inc:"); l3->setStyleSheet(lblStyle); form->addRow(l3, cmbDegInc);
    QLabel *l4 = new QLabel("DEG/S:"); l4->setStyleSheet(lblStyle); form->addRow(l4, spnDegs);
    QLabel *l5 = new QLabel("FRAME:"); l5->setStyleSheet(lblStyle); form->addRow(l5, cmbFrame);

    QWidget *speedWidget = new QWidget();
    QVBoxLayout *speedLayout = new QVBoxLayout(speedWidget);
    speedLayout->setAlignment(Qt::AlignCenter);
    speedLayout->setSpacing(20);

    QLabel *lblTitle = new QLabel("GLOBAL SPEED");
    lblTitle->setStyleSheet("color: white; font-weight: bold; font-size: 16px; letter-spacing: 2px;");
    lblTitle->setAlignment(Qt::AlignCenter);

    QLabel *lblReadout = new QLabel("50%");
    lblReadout->setStyleSheet("color: #0EA5E9; font-weight: bold; font-size: 32px; border: 2px solid #0EA5E9; border-radius: 8px; padding: 10px 30px; background-color: #1E1E24;");
    lblReadout->setAlignment(Qt::AlignCenter);

    QSlider *speedSlider = new QSlider(Qt::Horizontal);
    speedSlider->setRange(1, 100);
    speedSlider->setValue(50);
    speedSlider->setStyleSheet("QSlider::groove:horizontal { border-radius: 4px; height: 8px; background: #2D2D30; } QSlider::sub-page:horizontal { background: qlineargradient(x1: 0, y1: 0, x2: 1, y2: 0, stop: 0 #22C55E, stop: 1 #0EA5E9); border-radius: 4px; } QSlider::handle:horizontal { background: white; width: 18px; height: 18px; margin: -5px 0; border-radius: 9px; }");

    connect(speedSlider, &QSlider::valueChanged, [lblReadout](int value){ lblReadout->setText(QString::number(value) + "%"); });

    speedLayout->addWidget(lblTitle);
    speedLayout->addWidget(lblReadout, 0, Qt::AlignCenter);
    speedLayout->addWidget(speedSlider);

    mainLayout->addWidget(formWidget, 1);
    QFrame *vLine = new QFrame(); vLine->setFrameShape(QFrame::VLine); vLine->setStyleSheet("background-color: #3E3E42; max-width: 1px; border: none;");
    mainLayout->addWidget(vLine);
    mainLayout->addWidget(speedWidget, 1);

    return panel;
}