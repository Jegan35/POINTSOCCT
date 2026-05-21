#include "LeftPanel.h"
#include <QGridLayout>
#include <QTimer> // Needed for the auto-load delay
#include <QMenu>
#include <QAction>

LeftPanel::LeftPanel(QWidget *parent) : QWidget(parent)
{
    setupUI();
}

void LeftPanel::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    mainLayout->setSpacing(5);

    // ==========================================
    // MIDDLE SECTION: 3D Canvas (Left) + Joints (Right)
    // ==========================================
    QHBoxLayout *midLayout = new QHBoxLayout();

    // The OCCT Widget
    myMainWidget = new OcctWidget(this);
    myMainWidget->setViewRole(OcctWidget::MainRole);
    midLayout->addWidget(myMainWidget, 4); // Takes up 80%

    // Joint Readout Column
    QVBoxLayout *jointLayout = new QVBoxLayout();
    jointLayout->setSpacing(2);
    QLabel *jHeader = new QLabel("JOINTS", this);
    jHeader->setStyleSheet("background-color: #00BFFF; color: black; font-weight: bold; padding: 10px;");
    jHeader->setAlignment(Qt::AlignCenter);
    jointLayout->addWidget(jHeader);

    for(int i = 1; i <= 6; i++) {
        QLabel *jLbl = new QLabel(QString("J%1\n0.000°").arg(i), this);
        jLbl->setStyleSheet("background-color: #00BFFF; color: black; font-weight: bold; padding: 15px; border: 1px solid #1E1E1E;");
        jLbl->setAlignment(Qt::AlignCenter);
        jointLayout->addWidget(jLbl);
    }
    midLayout->addLayout(jointLayout, 1); // Takes up 20%
    mainLayout->addLayout(midLayout, 1);

    // ==========================================
    // BOTTOM SECTION 1: Coordinates
    // ==========================================
    QHBoxLayout *coordLayout = new QHBoxLayout();

    QLabel *lblAxis = new QLabel("AXIS GFX", this);
    lblAxis->setStyleSheet("background-color: #00BFFF; color: black; font-weight: bold; padding: 20px;");
    coordLayout->addWidget(lblAxis, 1);

    lblXYZ = new QLabel("X  0.000 mm\nY  0.000 mm\nZ  0.000 mm", this);
    lblXYZ->setStyleSheet("background-color: #00BFFF; color: black; font-weight: bold; padding: 10px; font-size: 16px;");
    coordLayout->addWidget(lblXYZ, 2);

    lblABC = new QLabel("A  0.000 °\nB  0.000 °\nC  0.000 °", this);
    lblABC->setStyleSheet("background-color: #00BFFF; color: black; font-weight: bold; padding: 10px; font-size: 16px;");
    coordLayout->addWidget(lblABC, 2);

    mainLayout->addLayout(coordLayout);

    // ==========================================
    // BOTTOM SECTION 2: 2-Row Footer Buttons
    // ==========================================
    QGridLayout *footerGrid = new QGridLayout();
    footerGrid->setSpacing(5);

    QString baseBtn = "color: white; font-weight: bold; font-size: 14px; min-height: 45px; border-radius: 3px;";

    // --- ROW 1 (6 Buttons) ---
    QPushButton *btnServo = new QPushButton("OFF", this);
    btnServo->setStyleSheet("background-color: #4B5563; " + baseBtn); // Grey
    connect(btnServo, &QPushButton::clicked, [btnServo, baseBtn]() {
        if(btnServo->text() == "OFF") { btnServo->setText("ON"); btnServo->setStyleSheet("background-color: #10B981; " + baseBtn); }
        else { btnServo->setText("OFF"); btnServo->setStyleSheet("background-color: #4B5563; " + baseBtn); }
    });

    QPushButton *btnHome = new QPushButton("HOME", this);
    btnHome->setStyleSheet("background-color: #007ACC; " + baseBtn); // Blue

    QPushButton *btnRun = new QPushButton("RUN", this);
    btnRun->setStyleSheet("background-color: #10B981; " + baseBtn); // Green
    connect(btnRun, &QPushButton::clicked, [btnRun, baseBtn]() {
        if(btnRun->text() == "RUN") { btnRun->setText("PAUSE"); btnRun->setStyleSheet("background-color: #F59E0B; " + baseBtn); }
        else { btnRun->setText("RUN"); btnRun->setStyleSheet("background-color: #10B981; " + baseBtn); }
    });

    QPushButton *btnStart = new QPushButton("START", this);
    btnStart->setStyleSheet("background-color: #4B5563; " + baseBtn); // Grey
    connect(btnStart, &QPushButton::clicked, [btnStart, baseBtn]() {
        if(btnStart->text() == "START") { btnStart->setText("STOP"); btnStart->setStyleSheet("background-color: #EF4444; " + baseBtn); }
        else { btnStart->setText("START"); btnStart->setStyleSheet("background-color: #4B5563; " + baseBtn); }
    });

    QPushButton *btnExit = new QPushButton("EXIT", this);
    btnExit->setStyleSheet("background-color: #991B1B; " + baseBtn); // Dark Red

    // ... previous buttons (btnExit, btnStart, etc) ...

    // --- NEW POPUP MENU BUTTON ---
    QPushButton *btnMode = new QPushButton("M: SIM", this);

    // The "menu-indicator: none" hides the ugly default Qt dropdown arrow so it looks like a clean button
    QString btnModeStyle = baseBtn + " QPushButton::menu-indicator { image: none; }";
    btnMode->setStyleSheet("background-color: #059669; " + btnModeStyle);

    // Create the dropdown menu
    QMenu *modeMenu = new QMenu(btnMode);
    modeMenu->setStyleSheet("QMenu { background-color: #2D2D30; color: white; border: 1px solid #3E3E42; font-weight: bold; font-size: 14px; } "
                            "QMenu::item { padding: 10px 30px; } "
                            "QMenu::item:selected { background-color: #007ACC; }");

    // Create the options
    QAction *actSim = new QAction("M: SIM", this);
    QAction *actReal = new QAction("M: REAL", this);

    modeMenu->addAction(actSim);
    modeMenu->addAction(actReal);

    // Attach the menu to the button
    btnMode->setMenu(modeMenu);

    // Wire up the clicks to change the button text and color
    connect(actSim, &QAction::triggered, [btnMode, btnModeStyle]() {
        btnMode->setText("M: SIM");
        btnMode->setStyleSheet("background-color: #059669; " + btnModeStyle); // Green
    });

    connect(actReal, &QAction::triggered, [btnMode, btnModeStyle]() {
        btnMode->setText("M: REAL");
        btnMode->setStyleSheet("background-color: #D97706; " + btnModeStyle); // Orange
    });

    // Add Row 1 to Grid
    footerGrid->addWidget(btnServo, 0, 0);
    // ... rest of your grid additions ...
    footerGrid->addWidget(btnHome,  0, 1);
    footerGrid->addWidget(btnRun,   0, 2);
    footerGrid->addWidget(btnStart, 0, 3);
    footerGrid->addWidget(btnExit,  0, 4);
    footerGrid->addWidget(btnMode,  0, 5);

    // --- ROW 2 (5 Elements, First takes 2 spaces) ---
    QPushButton *btnSysOk = new QPushButton("SYSTEM OK", this);
    btnSysOk->setStyleSheet("background-color: #10B981; " + baseBtn); // Green

    QPushButton *btnErrClr = new QPushButton("ERRCLR", this);
    btnErrClr->setStyleSheet("background-color: #EF4444; " + baseBtn); // Red

    QPushButton *btnMrkClr = new QPushButton("MRKCLR", this);
    btnMrkClr->setStyleSheet("background-color: #F59E0B; " + baseBtn); // Orange

    QPushButton *btnReset = new QPushButton("RESET", this);
    btnReset->setStyleSheet("background-color: #DC2626; " + baseBtn); // Red

    QPushButton *btnSpeed = new QPushButton("SPEED", this);
    btnSpeed->setStyleSheet("background-color: #064E3B; " + baseBtn); // Very Dark Green

    // Add Row 2 to Grid
    footerGrid->addWidget(btnSysOk,  1, 0, 1, 2); // Row 1, Col 0, spans 1 row, 2 columns
    footerGrid->addWidget(btnErrClr, 1, 2);
    footerGrid->addWidget(btnMrkClr, 1, 3);
    footerGrid->addWidget(btnReset,  1, 4);
    footerGrid->addWidget(btnSpeed,  1, 5);

    mainLayout->addLayout(footerGrid);

    // ==========================================
    // THE FIX: SAFE AUTO-LOAD TRIGGER
    // ==========================================
    // Wait 500ms for Linux/X11 to fully build the screen before loading the STLs
    QTimer::singleShot(500, myMainWidget, &OcctWidget::loadDefaultRobot);
}