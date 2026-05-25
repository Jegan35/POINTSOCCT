#include "LeftPanel.h"
#include <QGridLayout>
#include <QTimer>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QMouseEvent>
#include <QEvent>
#include <QDialog>
#include <QLineEdit>
#include <QListWidget>
#include <cmath>
#include "StepControl.h"
// ─────────────────────────────────────────────────────────────────────────────
//  DESIGN SYSTEM
// ─────────────────────────────────────────────────────────────────────────────
namespace DS {

static const char* PANEL_BG        = "#0D0F14";
static const char* CYAN_BRIGHT     = "#00E5FF";
static const char* CYAN_DIM        = "#00A3CC";
static const char* GREEN_BRIGHT    = "#00FF9D";
static const char* GREEN_DIM       = "#00B36E";
static const char* AMBER_BRIGHT    = "#FFC107";
static const char* AMBER_DIM       = "#D97706";
static const char* RED_BRIGHT      = "#FF4444";
static const char* RED_DIM         = "#CC1111";
static const char* BLUE_BRIGHT     = "#4D9BFF";
static const char* BLUE_DIM        = "#1D66D6";
static const char* VIOLET_BRIGHT   = "#A855F7";
static const char* VIOLET_DIM      = "#7E22CE";
static const char* SLATE           = "#64748B";
static const char* SLATE_DIM       = "#475569";
static const char* EMERALD_BRIGHT  = "#10DC8E";
static const char* EMERALD_DIM     = "#059669";
static const char* TEXT_PRIMARY    = "#FFFFFF";
static const char* TEXT_SECONDARY  = "#A0AEC0";

static const QString BTN_FONT =
    "color: #FFFFFF; font-family: 'Rajdhani', 'Consolas', monospace; "
    "font-weight: 900; font-size: 14px; letter-spacing: 1.5px; "
    "min-height: 48px; border-radius: 4px; ";

static QString raisedBtn(const QString& topColor,
                         const QString& rimColor,
                         const QString& glowColor = "#FFFFFF18")
{
    return QString(
               "QPushButton {"
               "  background-color: %1;"
               "  border-top: 1px solid %3;"
               "  border-left: 1px solid %3;"
               "  border-right: 1px solid %2;"
               "  border-bottom: 4px solid %2;"
               "  %4"
               "}"
               "QPushButton:hover {"
               "  background-color: %1;"
               "}"
               "QPushButton:pressed {"
               "  border-bottom: 1px solid %2;"
               "  border-top: 4px solid %2;"
               "  margin-top: 3px;"
               "}"
               "QPushButton:disabled {"
               "  background-color: #1C2130;"
               "  color: #3A4460;"
               "  border-bottom: 2px solid #111520;"
               "}"
               ).arg(topColor, rimColor, glowColor, DS::BTN_FONT);
}

inline QString btnOff()     { return raisedBtn(DS::SLATE,          "#1E293B", "#FFFFFF20"); }
inline QString btnHome()    { return raisedBtn(DS::BLUE_BRIGHT,    DS::BLUE_DIM); }
inline QString btnRun()     { return raisedBtn(DS::EMERALD_BRIGHT, DS::EMERALD_DIM); }
inline QString btnPause()   { return raisedBtn(DS::AMBER_BRIGHT,   DS::AMBER_DIM); }
inline QString btnStart()   { return raisedBtn(DS::SLATE,          DS::SLATE_DIM); }
inline QString btnStop()    { return raisedBtn(DS::RED_BRIGHT,     DS::RED_DIM); }
inline QString btnExit()    { return raisedBtn("#FF2A2A",          "#B31212"); }
inline QString btnSim()     { return raisedBtn(DS::VIOLET_BRIGHT,  DS::VIOLET_DIM); }
inline QString btnReal()    { return raisedBtn("#FF8C00",          "#B35F00"); }
inline QString btnOK()      { return raisedBtn(DS::EMERALD_BRIGHT, DS::GREEN_DIM); }
inline QString btnError()   { return raisedBtn(DS::RED_BRIGHT,     DS::RED_DIM); }
inline QString btnErrClr()  { return raisedBtn("#FF3366",          "#CC1133"); }
inline QString btnMrkClr()  { return raisedBtn(DS::AMBER_BRIGHT,   DS::AMBER_DIM); }
inline QString btnReset()   { return raisedBtn("#FF5555",          "#CC2222"); }
inline QString btnFiles()   { return raisedBtn(DS::VIOLET_BRIGHT,  DS::VIOLET_DIM); }
inline QString btnTools()   { return raisedBtn("#FF007F",          "#B30059"); }

inline QString btnAction()  {
    return "QPushButton {"
           "  background-color: #1C2130;"
           "  color: #FFFFFF;"
           "  font-family: 'Rajdhani','Consolas',monospace;"
           "  font-weight: 800; font-size: 13px; letter-spacing: 1px;"
           "  min-height: 44px; border-radius: 4px;"
           "  border: 1px solid #3A4460;"
           "  border-bottom: 3px solid #0D0F14;"
           "}"
           "QPushButton:hover { background-color: #252C3E; color: #00E5FF; }"
           "QPushButton:pressed { border-bottom: 1px solid #0D0F14; border-top: 3px solid #0D0F14; margin-top: 2px; }";
}

inline QString darkBox() {
    return "QLabel {"
           "  background-color: #141820;"
           "  color: #8A9AB8;"
           "  font-family: 'Rajdhani','Consolas',monospace;"
           "  font-weight: 600; font-size: 12px; letter-spacing: 0.8px;"
           "  padding: 10px 8px; border-radius: 4px;"
           "  border: 1px solid #252C3E;"
           "}";
}
inline QString prBox() {
    return "QLabel {"
           "  background-color: #141820;"
           "  color: #00D4FF;"
           "  font-family: 'Rajdhani','Consolas',monospace;"
           "  font-weight: 600; font-size: 12px; letter-spacing: 0.8px;"
           "  padding: 10px 8px; border-radius: 4px;"
           "  border: 1px solid #007A99; border-top: 1px solid #00D4FF;"
           "}";
}

inline QString jointLbl() {
    return "QLabel {"
           "  background-color: #00BFFF;"
           "  color: black;"
           "  font-family: 'Rajdhani','Consolas',monospace;"
           "  font-weight: 700; font-size: 12px; letter-spacing: 0.5px;"
           "  padding: 14px 8px; border-radius: 0px;"
           "  border: 1px solid #1E1E1E;"
           "}";
}

inline QString coordLbl(int pxSize = 15) {
    return QString(
               "QLabel {"
               "  background-color: #00BFFF;"
               "  color: black;"
               "  font-family: 'Rajdhani','Consolas',monospace;"
               "  font-weight: 700; font-size: %1px; letter-spacing: 0.5px;"
               "  padding: 10px 12px; border-radius: 0px;"
               "  border: 1px solid #1E1E1E;"
               "}"
               ).arg(pxSize);
}

inline QString jointHeader() {
    return "QLabel {"
           "  background-color: #00BFFF;"
           "  color: black;"
           "  font-family: 'Rajdhani','Consolas',monospace;"
           "  font-weight: 800; font-size: 11px; letter-spacing: 2px;"
           "  padding: 10px 8px; border-radius: 0px;"
           "  border: 1px solid #1E1E1E;"
           "}";
}

inline QString axisLbl() {
    return "QLabel {"
           "  background-color: #00BFFF;"
           "  color: black;"
           "  font-family: 'Rajdhani','Consolas',monospace;"
           "  font-weight: 800; font-size: 11px; letter-spacing: 2px;"
           "  padding: 20px 10px; border-radius: 0px;"
           "  border: 1px solid #1E1E1E;"
           "}";
}

inline QString speedBadge() {
    return "QLabel {"
           "  background-color: #0C1020;"
           "  color: #00D4FF;"
           "  font-family: 'Consolas',monospace;"
           "  font-weight: 700; font-size: 12px; letter-spacing: 1px;"
           "  padding: 8px 12px; border-radius: 3px;"
           "  border: 1px solid #007A99;"
           "}";
}

} // namespace DS


// ─────────────────────────────────────────────────────────────────────────────
//  LeftPanel constructor
// ─────────────────────────────────────────────────────────────────────────────
LeftPanel::LeftPanel(ClientBackend *backend, QWidget *parent)
    : QWidget(parent), m_backend(backend)
{
    setStyleSheet(QString("LeftPanel { background-color: %1; }").arg(DS::PANEL_BG));
    setupUI();
    m_uiThrottleTimer.start();

    if (m_backend) {
        connect(m_backend, &ClientBackend::telemetryChanged, this, &LeftPanel::updateTelemetryUI);
        connect(m_backend, &ClientBackend::directoryDataChanged, this, &LeftPanel::updateDirectoryUI);
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  setupUI
// ─────────────────────────────────────────────────────────────────────────────
void LeftPanel::setupUI()
{
    // ── Root layout for LeftPanel ──────────────────────────────────
    // Declared as local — topArea + footerStack are added directly.
    // NEVER reassign this pointer inside lambdas or button handlers!
    QVBoxLayout *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(4, 4, 4, 20);
    rootLayout->setSpacing(2);

    // ══════════════════════════════════════════════════════════════
    //  TOP AREA — 3D view + joints + coord readouts
    //  stretch=1 → takes ALL space not claimed by the footer
    // ══════════════════════════════════════════════════════════════
    QWidget *topArea = new QWidget(this);
    topArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *topAreaLayout = new QVBoxLayout(topArea);
    topAreaLayout->setContentsMargins(0, 0, 0, 0);
    topAreaLayout->setSpacing(2);

    // ── 3D Canvas (80%) + Joint column (20%) ──────────────────────
    QHBoxLayout *midLayout = new QHBoxLayout();
    midLayout->setSpacing(6);

    QFrame *occtContainer = new QFrame(topArea);
    occtContainer->setObjectName("occtContainer");
    occtContainer->setStyleSheet(
        "#occtContainer { background-color: #08090E; border: 1px solid #007A99; border-radius: 3px; }");
    occtContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    occtContainer->setMinimumSize(100, 100);
    occtContainer->setAttribute(Qt::WA_NativeWindow);

    QVBoxLayout *containerLay = new QVBoxLayout(occtContainer);
    containerLay->setContentsMargins(1, 1, 1, 1);
    containerLay->setSpacing(0);

    myMainWidget = new OcctWidget(occtContainer);
    myMainWidget->setViewRole(OcctWidget::MainRole);
    myMainWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    myMainWidget->setMinimumSize(100, 100);
    containerLay->addWidget(myMainWidget);

    midLayout->addWidget(occtContainer, 4);  // 80%

    connect(myMainWidget, &OcctWidget::selectionChanged,
            this, &LeftPanel::partSelectionStateChanged);

    // ── Joint column ──────────────────────────────────────────────
    QVBoxLayout *jointLayout = new QVBoxLayout();
    jointLayout->setSpacing(3);

    QLabel *jHeader = new QLabel("JOINTS", topArea);
    jHeader->setStyleSheet(DS::jointHeader());
    jHeader->setAlignment(Qt::AlignCenter);
    jointLayout->addWidget(jHeader);

    for (int i = 0; i < 6; i++) {
        m_lblJoints[i] = new QLabel(QString("J%1\n0.000°").arg(i + 1), topArea);
        m_lblJoints[i]->setStyleSheet(DS::jointLbl());
        m_lblJoints[i]->setAlignment(Qt::AlignCenter);
        jointLayout->addWidget(m_lblJoints[i]);
    }
    midLayout->addLayout(jointLayout, 1);    // 20%

    // ── Coordinate readouts ───────────────────────────────────────
    QHBoxLayout *coordLayout = new QHBoxLayout();
    coordLayout->setSpacing(4);

    QLabel *lblAxis = new QLabel("AXIS\nGFX", topArea);
    lblAxis->setStyleSheet(DS::axisLbl());
    lblAxis->setAlignment(Qt::AlignCenter);
    coordLayout->addWidget(lblAxis, 1);

    lblXYZ = new QLabel("X  0.000 mm\nY  0.000 mm\nZ  0.000 mm", topArea);
    lblXYZ->setStyleSheet(DS::coordLbl(15));
    coordLayout->addWidget(lblXYZ, 2);

    lblABC = new QLabel("A  0.000 °\nB  0.000 °\nC  0.000 °", topArea);
    lblABC->setStyleSheet(DS::coordLbl(15));
    coordLayout->addWidget(lblABC, 2);

    topAreaLayout->addLayout(midLayout,   1);   // 3D view expands
    topAreaLayout->addLayout(coordLayout, 0);   // coord bar compact

    rootLayout->addWidget(topArea, 1);           // topArea takes all non-footer space

    // ══════════════════════════════════════════════════════════════
    //  FOOTER STACK — strictly frozen, nothing can disturb it
    // ══════════════════════════════════════════════════════════════
    footerStack = new QStackedWidget(this);

    // ─────────────────────────────────────────────────────────────
    //  PAGE 0 – Primary controls
    // ─────────────────────────────────────────────────────────────
    QWidget *page0 = new QWidget();
    QGridLayout *footerGrid = new QGridLayout(page0);
    footerGrid->setSpacing(4);
    footerGrid->setContentsMargins(0, 4, 0, 0);

    m_btnServo = new QPushButton("⬤  OFF", this);
    m_btnServo->setStyleSheet(DS::btnOff());
    connect(m_btnServo, &QPushButton::clicked, [this]() {
        if (m_backend) m_backend->toggleServo();
    });

    QPushButton *btnHome = new QPushButton("⌂  HOME", this);
    btnHome->setStyleSheet(DS::btnHome());
    connect(btnHome, &QPushButton::clicked, [this]() {
        if (m_backend) m_backend->triggerHome();
    });

    m_btnRun = new QPushButton("▶  RUN", this);
    m_btnRun->setStyleSheet(DS::btnRun());
    connect(m_btnRun, &QPushButton::clicked, [this]() {
        if (m_backend) m_backend->toggleRunPause();
    });

    m_btnStart = new QPushButton("◼  START", this);
    m_btnStart->setStyleSheet(DS::btnStart());
    connect(m_btnStart, &QPushButton::clicked, [this]() {
        if (m_backend) m_backend->toggleStartStop();
    });

    QPushButton *btnExit = new QPushButton("✕  EXIT", this);
    btnExit->setStyleSheet(DS::btnExit());
    connect(btnExit, &QPushButton::clicked, [this]() {
        if (m_backend) m_backend->triggerExit();
    });

    m_btnMode = new QPushButton("M: SIM", this);
    m_btnMode->setStyleSheet(DS::btnSim() + " QPushButton::menu-indicator { image: none; }");

    QMenu *modeMenu = new QMenu(m_btnMode);
    modeMenu->setStyleSheet(
        "QMenu { background-color: #141820; color: #E8EDF5; border: 1px solid #3A4460;"
        "  font-family: 'Rajdhani','Consolas',monospace; font-weight: 700; font-size: 13px; }"
        "QMenu::item { padding: 10px 28px; }"
        "QMenu::item:selected { background-color: #007A99; color: #00D4FF; }");
    QAction *actSim  = new QAction("M: SIM",  this);
    QAction *actReal = new QAction("M: REAL", this);
    modeMenu->addAction(actSim);
    modeMenu->addAction(actReal);
    m_btnMode->setMenu(modeMenu);
    connect(actSim,  &QAction::triggered, [this]() { if (m_backend) m_backend->setSimMode();  });
    connect(actReal, &QAction::triggered, [this]() { if (m_backend) m_backend->setRealMode(); });

    footerGrid->addWidget(m_btnServo, 0, 0);
    footerGrid->addWidget(btnHome,    0, 1);
    footerGrid->addWidget(m_btnRun,   0, 2);
    footerGrid->addWidget(m_btnStart, 0, 3);
    footerGrid->addWidget(btnExit,    0, 4);
    footerGrid->addWidget(m_btnMode,  0, 5);

    // ── System health button ──────────────────────────────────────
    m_btnSysHealth = new QPushButton("●  SYSTEM OK", this);
    m_btnSysHealth->setStyleSheet(DS::btnOK());

    connect(m_btnSysHealth, &QPushButton::clicked, this, [this]() {
        // ── FIX: Use a completely local QVBoxLayout for the dialog frame.
        //         NEVER touch m_mainLayout or rootLayout here. ──────────
        QDialog dialog(this);
        dialog.setFixedSize(360, 160);
        dialog.setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
        dialog.setAttribute(Qt::WA_TranslucentBackground);
        dialog.setStyleSheet("background: transparent;");

        QFrame *mainFrame = new QFrame(&dialog);
        mainFrame->setObjectName("container");

        QString currentErr = m_backend ? m_backend->property("errorMessage").toString() : "";
        QString errLower   = currentErr.toLower().trimmed();
        bool isError = !currentErr.isEmpty()
                       && errLower != "no active errors"
                       && errLower != "no error"
                       && errLower != "none";
        if (!isError) currentErr = "SYSTEM IS OPERATIONAL";

        mainFrame->setStyleSheet(QString(
                                     "#container { background-color: #0d1117; border: 3px solid %1; border-radius: 8px; }"
                                     ).arg(isError ? "#FF5252" : "#22C55E"));

        // ── LOCAL layout for the frame — not m_mainLayout ──
        QVBoxLayout *frameLayout = new QVBoxLayout(mainFrame);
        frameLayout->setContentsMargins(0, 0, 0, 0);
        frameLayout->setSpacing(0);

        QWidget *headerBar = new QWidget();
        headerBar->setStyleSheet(
            "background-color: #1a1e2a; border-bottom: 1px solid #2a2d35;"
            "border-top-left-radius: 5px; border-top-right-radius: 5px;");
        QHBoxLayout *headerLay = new QHBoxLayout(headerBar);
        headerLay->setContentsMargins(12, 4, 5, 4);

        QLabel *title = new QLabel("SYSTEM STATUS");
        title->setStyleSheet(
            "color: #00bcd4; font-weight: bold; font-size: 10px;"
            "letter-spacing: 1px; background: transparent;");

        QPushButton *btnClose = new QPushButton("✖");
        btnClose->setFixedSize(24, 24);
        btnClose->setStyleSheet(
            "QPushButton { background: transparent; color: #888; font-size: 12px; border: none; }"
            "QPushButton:hover { color: #ff5252; }");
        connect(btnClose, &QPushButton::clicked, &dialog, &QDialog::close);

        headerLay->addWidget(title);
        headerLay->addStretch();
        headerLay->addWidget(btnClose);
        frameLayout->addWidget(headerBar);

        QLabel *lblMsg = new QLabel(currentErr);
        lblMsg->setStyleSheet(QString(
                                  "color: %1; font-family: 'Rajdhani', sans-serif;"
                                  "font-weight: 700; font-size: 14px; padding: 5px 10px 10px 10px; background: transparent;"
                                  ).arg(isError ? "#FF5252" : "#E8EDF5"));
        lblMsg->setAlignment(Qt::AlignHCenter);
        lblMsg->setWordWrap(true);
        frameLayout->addWidget(lblMsg, 0, Qt::AlignTop);
        frameLayout->addStretch();

        // ── Dialog outer layout ──
        QVBoxLayout *outer = new QVBoxLayout(&dialog);
        outer->setContentsMargins(0, 0, 0, 0);
        outer->addWidget(mainFrame);

        QPoint pos = m_btnSysHealth->mapToGlobal(QPoint(0, -dialog.height() - 12));
        dialog.move(pos);
        dialog.exec();
    });

    footerGrid->addWidget(m_btnSysHealth, 1, 0, 1, 2);

    QPushButton *btnErrClr = new QPushButton("✕  ERRCLR", this);
    btnErrClr->setStyleSheet(DS::btnErrClr());
    connect(btnErrClr, &QPushButton::clicked, [this]() {
        if (m_backend) {
            m_backend->clearErrors();
            m_backend->setProperty("errorMessage", "No Error");
            m_backend->setProperty("isEmergency", false);
        }
        updateTelemetryUI();
    });

    QPushButton *btnMrkClr = new QPushButton("◈  MRKCLR", this);
    btnMrkClr->setStyleSheet(DS::btnMrkClr());
    connect(btnMrkClr, &QPushButton::clicked, [this]() {
        if (m_backend) m_backend->clearMarks();
        if (myMainWidget) myMainWidget->clearMarks();
    });

    QPushButton *btnReset = new QPushButton("↺  RESET", this);
    btnReset->setStyleSheet(DS::btnReset());

    QLabel *lblSpeed = new QLabel("S: 0.0 %", this);
    lblSpeed->setStyleSheet(DS::speedBadge());
    lblSpeed->setAlignment(Qt::AlignCenter);

    footerGrid->addWidget(btnErrClr, 1, 2);
    footerGrid->addWidget(btnMrkClr, 1, 3);
    footerGrid->addWidget(btnReset,  1, 4);
    footerGrid->addWidget(lblSpeed,  1, 5);

    // ─────────────────────────────────────────────────────────────
    //  PAGE 1 – Action / Tools grid
    // ─────────────────────────────────────────────────────────────
    QWidget *page1 = new QWidget();
    QGridLayout *swipeGrid = new QGridLayout(page1);
    swipeGrid->setSpacing(4);
    swipeGrid->setContentsMargins(0, 4, 0, 0);

    QPushButton *btnFiles = new QPushButton("📂  FILES", page1);
    btnFiles->setStyleSheet(DS::btnFiles());

    connect(btnFiles, &QPushButton::clicked, this, [this, btnFiles]() {
        QDialog dialog(this);
        dialog.setFixedSize(350, 460);
        dialog.setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
        dialog.setStyleSheet(
            "QDialog { background-color: #141820; border: 1px solid #3A4460; border-radius: 8px; }");

        // ── FIX: purely local layout for the dialog — not m_mainLayout ──
        QVBoxLayout *dlgLayout = new QVBoxLayout(&dialog);
        dlgLayout->setContentsMargins(15, 15, 15, 15);
        QStackedWidget *stack = new QStackedWidget(&dialog);

        QString activeCategory = "TP";
        QString currentAction  = "OPEN";

        QString baseStyle =
            "QPushButton {"
            "  color: #E8EDF5; font-family: 'Rajdhani','Consolas',monospace;"
            "  font-weight: 700; font-size: 14px; letter-spacing: 1px;"
            "  border-radius: 5px; padding: 12px; min-height: 26px;"
            "}"
            "QPushButton:pressed { margin-top: 4px; }"
            "QPushButton:disabled { background-color: #1C2130; color: #3A4460; }";

        // PAGE 0 – type select
        QWidget *page0 = new QWidget();
        QVBoxLayout *p0Layout = new QVBoxLayout(page0);
        QLabel *lblTitle0 = new QLabel("SELECT TYPE");
        lblTitle0->setStyleSheet("color: #8A9AB8; font-family: 'Rajdhani','Consolas',monospace;"
                                 "font-weight: 700; font-size: 13px; letter-spacing: 1.5px; border: none;");
        lblTitle0->setAlignment(Qt::AlignCenter);
        QPushButton *btnTp  = new QPushButton("TARGET POINT FILES");
        btnTp->setStyleSheet(baseStyle + "QPushButton { background-color: #00897B; border-bottom: 4px solid #004D40; }");
        QPushButton *btnPr  = new QPushButton("PROGRAM FILES");
        btnPr->setStyleSheet(baseStyle + "QPushButton { background-color: #1565C0; border-bottom: 4px solid #0D3B80; }");
        QPushButton *btnTr  = new QPushButton("TRAJECTORY FILES");
        btnTr->setStyleSheet(baseStyle + "QPushButton { background-color: #6A1B9A; border-bottom: 4px solid #3A0B5A; }");
        QPushButton *btnClose = new QPushButton("CLOSE");
        btnClose->setStyleSheet(baseStyle + "QPushButton { background-color: #B71C1C; border-bottom: 4px solid #7A0000; }");
        p0Layout->addWidget(lblTitle0); p0Layout->addWidget(btnTp);
        p0Layout->addWidget(btnPr); p0Layout->addWidget(btnTr);
        p0Layout->addStretch(); p0Layout->addWidget(btnClose);

        // PAGE 1 – operations
        QWidget *page1dlg = new QWidget();
        QVBoxLayout *p1Layout = new QVBoxLayout(page1dlg);
        QLabel *lblTitle1 = new QLabel("OPERATIONS");
        lblTitle1->setStyleSheet("color: #8A9AB8; font-family: 'Rajdhani','Consolas',monospace;"
                                 "font-weight: 700; font-size: 13px; letter-spacing: 1.5px; border: none;");
        lblTitle1->setAlignment(Qt::AlignCenter);
        QPushButton *btnNew    = new QPushButton("NEW FILE");
        btnNew->setStyleSheet(baseStyle + "QPushButton { background-color: #2E7D32; border-bottom: 4px solid #1B5E20; }");
        QPushButton *btnOpen   = new QPushButton("OPEN FILE");
        btnOpen->setStyleSheet(baseStyle + "QPushButton { background-color: #1565C0; border-bottom: 4px solid #0D3B80; }");
        QPushButton *btnDelete = new QPushButton("DELETE FILE");
        btnDelete->setStyleSheet(baseStyle + "QPushButton { background-color: #B71C1C; border-bottom: 4px solid #7A0000; }");
        QPushButton *btnBack1  = new QPushButton("BACK");
        btnBack1->setStyleSheet(baseStyle + "QPushButton { background-color: #2A3040; border-bottom: 4px solid #141820; }");
        p1Layout->addWidget(lblTitle1); p1Layout->addWidget(btnNew);
        p1Layout->addWidget(btnOpen); p1Layout->addWidget(btnDelete);
        p1Layout->addStretch(); p1Layout->addWidget(btnBack1);

        // PAGE 2 – create new
        QWidget *page2 = new QWidget();
        QVBoxLayout *p2Layout = new QVBoxLayout(page2);
        QLabel *lblTitle2 = new QLabel("CREATE NEW FILE");
        lblTitle2->setStyleSheet("color: #00E88A; font-family: 'Rajdhani','Consolas',monospace;"
                                 "font-weight: 700; font-size: 13px; letter-spacing: 1.5px; border: none;");
        lblTitle2->setAlignment(Qt::AlignCenter);
        QLabel *lblInputBox = new QLabel("FILENAME");
        lblInputBox->setStyleSheet("color: #3A4460; font-family: 'Rajdhani','Consolas',monospace;"
                                   "font-weight: 600; font-size: 10px; letter-spacing: 1px; border: none;");
        QLineEdit *txtFilename = new QLineEdit();
        txtFilename->setPlaceholderText("e.g., Weld_Path_01");
        txtFilename->setStyleSheet(
            "QLineEdit { background-color: #08090E; color: #E8EDF5; border: 1px solid #3A4460;"
            "  border-radius: 4px; padding: 12px;"
            "  font-family: 'Consolas',monospace; font-size: 15px; }"
            "QLineEdit:focus { border: 1px solid #00E88A; }");
        QHBoxLayout *p2BtnLayout = new QHBoxLayout();
        QPushButton *btnCancelCreate = new QPushButton("CANCEL");
        btnCancelCreate->setStyleSheet(baseStyle + "QPushButton { background-color: #B71C1C; border-bottom: 4px solid #7A0000; padding: 10px; }");
        QPushButton *btnSubmitCreate = new QPushButton("CREATE");
        btnSubmitCreate->setStyleSheet(baseStyle + "QPushButton { background-color: #2E7D32; border-bottom: 4px solid #1B5E20; padding: 10px; }");
        btnSubmitCreate->setEnabled(false);
        connect(txtFilename, &QLineEdit::textChanged,
                [btnSubmitCreate](const QString &text) { btnSubmitCreate->setEnabled(!text.trimmed().isEmpty()); });
        connect(btnSubmitCreate, &QPushButton::clicked, [&]() {
            if (m_backend) {
                if (activeCategory == "TP") m_backend->handleNewTp(txtFilename->text());
                else if (activeCategory == "PR") m_backend->handleNewPr(txtFilename->text());
            }
            dialog.accept();
        });
        p2BtnLayout->addWidget(btnCancelCreate); p2BtnLayout->addWidget(btnSubmitCreate);
        p2Layout->addWidget(lblTitle2); p2Layout->addWidget(lblInputBox);
        p2Layout->addWidget(txtFilename); p2Layout->addStretch();
        p2Layout->addLayout(p2BtnLayout);

        // PAGE 3 – file list
        QWidget *page3 = new QWidget();
        QVBoxLayout *p3Layout = new QVBoxLayout(page3);
        QLabel *lblTitle3 = new QLabel("SELECT FILE");
        lblTitle3->setStyleSheet("color: #00D4FF; font-family: 'Rajdhani','Consolas',monospace;"
                                 "font-weight: 700; font-size: 13px; letter-spacing: 1.5px; border: none;");
        lblTitle3->setAlignment(Qt::AlignCenter);

        QHBoxLayout *searchLayout = new QHBoxLayout();
        searchLayout->setSpacing(4);
        QLineEdit *txtSearch = new QLineEdit();
        txtSearch->setPlaceholderText("🔍 Search...");
        txtSearch->setStyleSheet(
            "QLineEdit { background-color: #08090E; color: #E8EDF5; border: 1px solid #3A4460;"
            "  border-radius: 4px; padding: 8px 12px; font-size: 13px; }"
            "QLineEdit:focus { border: 1px solid #00D4FF; }");
        QPushButton *btnRefreshList = new QPushButton("↺");
        btnRefreshList->setFixedSize(36, 36);
        btnRefreshList->setStyleSheet(
            "QPushButton { background-color: #1C2130; color: #8A9AB8; font-size: 16px;"
            "  border-radius: 4px; border: 1px solid #3A4460; }"
            "QPushButton:pressed { background-color: #252C3E; }");
        searchLayout->addWidget(txtSearch);
        searchLayout->addWidget(btnRefreshList);

        QListWidget *fileList = new QListWidget();
        fileList->setStyleSheet(
            "QListWidget { background-color: #08090E; color: #E8EDF5; border: 1px solid #252C3E;"
            "  border-radius: 4px; padding: 4px;"
            "  font-family: 'Consolas',monospace; font-size: 13px; }"
            "QListWidget::item { padding: 10px; border-bottom: 1px solid #1C2130; }"
            "QListWidget::item:selected { background-color: #003A55; color: #00D4FF; border-radius: 3px; }"
            "QListWidget::indicator { width: 22px; height: 22px; }");
        connect(txtSearch, &QLineEdit::textChanged, [fileList](const QString &text) {
            for (int i = 0; i < fileList->count(); ++i)
                fileList->item(i)->setHidden(!fileList->item(i)->text().contains(text, Qt::CaseInsensitive));
        });

        QHBoxLayout *p3BtnLayout = new QHBoxLayout();
        QPushButton *btnCancelList = new QPushButton("CANCEL");
        btnCancelList->setStyleSheet(baseStyle + "QPushButton { background-color: #B71C1C; border-bottom: 4px solid #7A0000; padding: 10px; }");
        QPushButton *btnSubmitList = new QPushButton("ACTION");
        btnSubmitList->setStyleSheet(baseStyle + "QPushButton { background-color: #1565C0; border-bottom: 4px solid #0D3B80; padding: 10px; }");
        btnSubmitList->setEnabled(false);

        auto checkSubmitState = [btnSubmitList, fileList, &currentAction]() {
            if (currentAction == "OPEN") {
                btnSubmitList->setEnabled(fileList->selectedItems().count() > 0);
            } else if (currentAction == "DELETE") {
                bool anyChecked = false;
                for (int i = 0; i < fileList->count(); ++i)
                    if (fileList->item(i)->checkState() == Qt::Checked) { anyChecked = true; break; }
                btnSubmitList->setEnabled(anyChecked);
            }
        };
        connect(fileList, &QListWidget::itemSelectionChanged, checkSubmitState);
        connect(fileList, &QListWidget::itemChanged, checkSubmitState);

        connect(btnSubmitList, &QPushButton::clicked, [&]() {
            if (!m_backend) return;
            if (currentAction == "OPEN" && fileList->currentItem()) {
                QString selectedFile = fileList->currentItem()->text().split('|').first().trimmed();
                if (activeCategory == "TP") { m_backend->handleOpenTp(selectedFile); emit requestTabChange(2); }
                else if (activeCategory == "PR") { m_backend->handleOpenPr(selectedFile); emit requestTabChange(0); }
            } else if (currentAction == "DELETE") {
                for (int i = 0; i < fileList->count(); ++i) {
                    QListWidgetItem *item = fileList->item(i);
                    if (item->checkState() == Qt::Checked) {
                        QString selectedFile = item->text().split('|').first().trimmed();
                        if (activeCategory == "TP") m_backend->handleDeleteTp(selectedFile);
                        else if (activeCategory == "PR") m_backend->handleDeletePr(selectedFile);
                    }
                }
            }
            dialog.accept();
        });

        p3BtnLayout->addWidget(btnCancelList); p3BtnLayout->addWidget(btnSubmitList);
        p3Layout->addWidget(lblTitle3);
        p3Layout->addLayout(searchLayout);
        p3Layout->addWidget(fileList);
        p3Layout->addLayout(p3BtnLayout);

        stack->addWidget(page0);
        stack->addWidget(page1dlg);   // ← renamed to avoid shadow with outer page1
        stack->addWidget(page2);
        stack->addWidget(page3);
        dlgLayout->addWidget(stack);  // ← FIX: was m_mainLayout->addWidget(stack)

        auto populateFiles = [&]() {
            fileList->clear();
            if (m_backend) {
                QStringList list;
                if (activeCategory == "TP") list = m_backend->property("tpFileList").toStringList();
                else if (activeCategory == "PR") list = m_backend->property("prFileList").toStringList();
                else if (activeCategory == "TR") list = m_backend->property("trFileList").toStringList();
                for (const QString &fileStr : list) {
                    QListWidgetItem *item = new QListWidgetItem(fileStr, fileList);
                    if (currentAction == "DELETE") {
                        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
                        item->setCheckState(Qt::Unchecked);
                    }
                }
            }
            btnSubmitList->setEnabled(false);
        };

        if (m_backend)
            connect(m_backend, &ClientBackend::directoryDataChanged, &dialog,
                    [&, stack, populateFiles]() { if (stack->currentIndex() == 3) populateFiles(); });

        connect(btnRefreshList, &QPushButton::clicked, [&]() {
            if (m_backend) {
                btnRefreshList->setText("…");
                QTimer::singleShot(200, &dialog, [btnRefreshList]() { btnRefreshList->setText("↺"); });
                if (activeCategory == "TP") m_backend->refreshTpFiles();
                else if (activeCategory == "PR") m_backend->refreshPrFiles();
            }
        });

        connect(btnTp, &QPushButton::clicked, [&]() { activeCategory = "TP"; lblTitle1->setText("TP OPERATIONS"); stack->setCurrentIndex(1); });
        connect(btnPr, &QPushButton::clicked, [&]() { activeCategory = "PR"; lblTitle1->setText("PR OPERATIONS"); stack->setCurrentIndex(1); });
        connect(btnTr, &QPushButton::clicked, [&]() { activeCategory = "TR"; lblTitle1->setText("TR OPERATIONS"); stack->setCurrentIndex(1); });
        connect(btnNew, &QPushButton::clicked, [&]() { lblTitle2->setText("CREATE NEW " + activeCategory); txtFilename->clear(); stack->setCurrentIndex(2); });

        connect(btnOpen, &QPushButton::clicked, [&]() {
            currentAction = "OPEN";
            lblTitle3->setText("OPEN " + activeCategory + " FILE");
            lblTitle3->setStyleSheet("color: #00D4FF; font-family: 'Rajdhani','Consolas',monospace; font-weight: 700; font-size: 13px; letter-spacing: 1.5px; border: none;");
            btnSubmitList->setText("OPEN");
            btnSubmitList->setStyleSheet(baseStyle + "QPushButton { background-color: #1565C0; border-bottom: 4px solid #0D3B80; padding: 10px; }");
            if (m_backend) { if (activeCategory == "TP") m_backend->refreshTpFiles(); else if (activeCategory == "PR") m_backend->refreshPrFiles(); }
            populateFiles(); stack->setCurrentIndex(3);
        });

        connect(btnDelete, &QPushButton::clicked, [&]() {
            currentAction = "DELETE";
            lblTitle3->setText("DELETE " + activeCategory + " FILE(S)");
            lblTitle3->setStyleSheet("color: #FF3B3B; font-family: 'Rajdhani','Consolas',monospace; font-weight: 700; font-size: 13px; letter-spacing: 1.5px; border: none;");
            btnSubmitList->setText("DELETE");
            btnSubmitList->setStyleSheet(baseStyle + "QPushButton { background-color: #B71C1C; border-bottom: 4px solid #7A0000; padding: 10px; }");
            if (m_backend) { if (activeCategory == "TP") m_backend->refreshTpFiles(); else if (activeCategory == "PR") m_backend->refreshPrFiles(); }
            populateFiles(); stack->setCurrentIndex(3);
        });

        connect(btnClose,        &QPushButton::clicked, &dialog, &QDialog::reject);
        connect(btnBack1,        &QPushButton::clicked, [&]() { stack->setCurrentIndex(0); });
        connect(btnCancelCreate, &QPushButton::clicked, [&]() { stack->setCurrentIndex(1); });
        connect(btnCancelList,   &QPushButton::clicked, [&]() { stack->setCurrentIndex(1); });

        QPoint pos = btnFiles->mapToGlobal(QPoint(0, 0));
        dialog.setGeometry(pos.x(), pos.y() - 470, 350, 460);
        dialog.exec();
    });

    m_lblTP = new QLabel("TP: None", page1); m_lblTP->setStyleSheet(DS::darkBox());
    m_lblPR = new QLabel("PR: None", page1); m_lblPR->setStyleSheet(DS::prBox());
    m_lblTR = new QLabel("TR: None", page1); m_lblTR->setStyleSheet(DS::darkBox());

    QPushButton *btnTools = new QPushButton("⊕  TOOLS", page1);
    btnTools->setStyleSheet(DS::btnTools());

    QLabel *lblToolName = new QLabel("Tool Name...", page1);
    lblToolName->setStyleSheet(DS::darkBox());

    QPushButton *btnAct1 = new QPushButton("DXF FILES", page1);
    btnAct1->setStyleSheet(DS::btnAction());
    connect(btnAct1, &QPushButton::clicked, this, [this]() {
        StepControl stepDialog(myMainWidget, this);
        connect(&stepDialog, &StepControl::requestDxfTab, this, [this]() {
            emit requestTabChange(5);
        });
        stepDialog.exec();
    });

    QPushButton *btnAct2 = new QPushButton("ACTION 2", page1); btnAct2->setStyleSheet(DS::btnAction());
    QPushButton *btnAct3 = new QPushButton("ACTION 3", page1); btnAct3->setStyleSheet(DS::btnAction());
    QPushButton *btnAct4 = new QPushButton("ACTION 4", page1); btnAct4->setStyleSheet(DS::btnAction());
    QPushButton *btnAct5 = new QPushButton("ACTION 5", page1); btnAct5->setStyleSheet(DS::btnAction());
    QPushButton *btnAct6 = new QPushButton("ACTION 6", page1); btnAct6->setStyleSheet(DS::btnAction());

    swipeGrid->addWidget(btnFiles,    0, 0);
    swipeGrid->addWidget(m_lblTP,     0, 1);
    swipeGrid->addWidget(m_lblPR,     0, 2);
    swipeGrid->addWidget(m_lblTR,     0, 3);
    swipeGrid->addWidget(btnTools,    0, 4);
    swipeGrid->addWidget(lblToolName, 0, 5);
    swipeGrid->addWidget(btnAct1, 1, 0);
    swipeGrid->addWidget(btnAct2, 1, 1);
    swipeGrid->addWidget(btnAct3, 1, 2);
    swipeGrid->addWidget(btnAct4, 1, 3);
    swipeGrid->addWidget(btnAct5, 1, 4);
    swipeGrid->addWidget(btnAct6, 1, 5);

    footerStack->addWidget(page0);
    footerStack->addWidget(page1);

    // ── FOOTER LOCK: setFixedHeight + Fixed policy = nothing can disturb it ──
    footerStack->setFixedHeight(120);
    footerStack->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    rootLayout->addWidget(footerStack, 0);   // stretch=0 — footer never grows

    // ── Timers & event filter ─────────────────────────────────────
    QTimer::singleShot(500, myMainWidget, &OcctWidget::loadDefaultRobot);
    qApp->installEventFilter(this);
    dragStartPos = QPoint(-1, -1);

    QTimer *dummyTimer = new QTimer(this);
    connect(dummyTimer, &QTimer::timeout, this, [this]() {
        if (!this->isVisible()) return;
        if (m_backend && m_backend->property("currentMode").toString() == "Real") return;
        if (m_backend && !m_backend->property("isWebClientConnected").toBool()) {
            static double t = 0;
            t += 0.02;
            m_backend->setProperty("j1", std::sin(t) * 45.0);
            m_backend->setProperty("j2", std::cos(t) * 30.0 - 15.0);
            m_backend->setProperty("j3", std::sin(t * 1.5) * 40.0 + 20.0);
            m_backend->setProperty("j4", std::cos(t * 2.0) * 90.0);
            m_backend->setProperty("j5", std::sin(t) * 60.0);
            m_backend->setProperty("j6", t * 50.0);
            updateTelemetryUI();
        }
    });
    dummyTimer->start(50);

    connect(myMainWidget, &OcctWidget::robotLoadComplete, this, [this]() {
        QTimer::singleShot(50, this, [this]() {
            int left, top, right, bottom;
            this->layout()->getContentsMargins(&left, &top, &right, &bottom);
            this->layout()->setContentsMargins(left, top, right + 1, bottom);
            QTimer::singleShot(50, this, [this, left, top, right, bottom]() {
                this->layout()->setContentsMargins(left, top, right, bottom);
            });
        });
    });
}

// ─────────────────────────────────────────────────────────────────────────────
//  updateTelemetryUI
// ─────────────────────────────────────────────────────────────────────────────
void LeftPanel::updateTelemetryUI()
{
    if (!m_backend) return;
    if (m_uiThrottleTimer.elapsed() < 33) return;
    m_uiThrottleTimer.restart();

    lblXYZ->setText(QString("X  %1 mm\nY  %2 mm\nZ  %3 mm")
                        .arg(m_backend->property("x").toDouble(), 0, 'f', 3)
                        .arg(m_backend->property("y").toDouble(), 0, 'f', 3)
                        .arg(m_backend->property("z").toDouble(), 0, 'f', 3));
    lblABC->setText(QString("A  %1 °\nB  %2 °\nC  %3 °")
                        .arg(m_backend->property("a").toDouble(), 0, 'f', 3)
                        .arg(m_backend->property("b").toDouble(), 0, 'f', 3)
                        .arg(m_backend->property("c").toDouble(), 0, 'f', 3));

    for (int i = 0; i < 6; i++)
        m_lblJoints[i]->setText(
            QString("J%1\n%2°").arg(i + 1)
                .arg(m_backend->property(QString("j%1").arg(i + 1).toUtf8().constData()).toDouble(), 0, 'f', 3));

    if (myMainWidget) {
        const double d2r = 3.14159265358979323846 / 180.0;
        myMainWidget->updateRobotPosture(
            m_backend->property("j1").toDouble() * d2r,
            m_backend->property("j2").toDouble() * d2r,
            m_backend->property("j3").toDouble() * d2r,
            m_backend->property("j4").toDouble() * d2r,
            m_backend->property("j5").toDouble() * d2r,
            m_backend->property("j6").toDouble() * d2r);
    }

    if (m_backend->property("isServoOn").toBool()) {
        m_btnServo->setText("⬤  ON");
        m_btnServo->setStyleSheet(DS::btnOK());
    } else {
        m_btnServo->setText("⬤  OFF");
        m_btnServo->setStyleSheet(DS::btnOff());
    }

    if (m_backend->property("isRunning").toBool()) {
        m_btnRun->setText("⏸  PAUSE");
        m_btnRun->setStyleSheet(DS::btnPause());
    } else {
        m_btnRun->setText("▶  RUN");
        m_btnRun->setStyleSheet(DS::btnRun());
    }

    if (m_backend->property("isStarted").toBool()) {
        m_btnStart->setText("■  STOP");
        m_btnStart->setStyleSheet(DS::btnStop());
    } else {
        m_btnStart->setText("◼  START");
        m_btnStart->setStyleSheet(DS::btnStart());
    }

    QString currentMode  = m_backend->property("currentMode").toString();
    QString btnModeExtra = " QPushButton::menu-indicator { image: none; }";
    m_btnMode->setText("M: " + currentMode.toUpper());
    m_btnMode->setStyleSheet((currentMode == "Sim" ? DS::btnSim() : DS::btnReal()) + btnModeExtra);

    QString errorMsg  = m_backend->property("errorMessage").toString().trimmed();
    bool isEmergency  = m_backend->property("isEmergency").toBool();
    QString errLower  = errorMsg.toLower();
    bool hasError     = isEmergency || (!errorMsg.isEmpty()
                                    && errLower != "no active errors"
                                    && errLower != "no error"
                                    && errLower != "none");

    if (hasError) {
        m_btnSysHealth->setText("⚠  VIEW ERROR");
        m_btnSysHealth->setStyleSheet(DS::btnError());
    } else {
        m_btnSysHealth->setText("●  SYSTEM OK");
        m_btnSysHealth->setStyleSheet(DS::btnOK());
    }
}

// ─────────────────────────────────────────────────────────────────────────────
//  updateDirectoryUI
// ─────────────────────────────────────────────────────────────────────────────
void LeftPanel::updateDirectoryUI()
{
    if (!m_backend) return;

    auto formatFileName = [](const QString &fullString) {
        QString name = fullString.split('|').first().trimmed();
        if (name.isEmpty() || name == "None") return QString("None");
        if (name.length() > 12) name = name.left(10) + "…";
        return name;
    };

    m_lblTP->setText("TP: " + formatFileName(m_backend->property("currentTpName").toString()));
    m_lblPR->setText("PR: " + formatFileName(m_backend->property("currentPrName").toString()));
}

// ─────────────────────────────────────────────────────────────────────────────
//  Footer swipe / event filter
// ─────────────────────────────────────────────────────────────────────────────
void LeftPanel::toggleFooterSwipe()
{
    footerStack->setCurrentIndex(footerStack->currentIndex() == 0 ? 1 : 0);
}

void LeftPanel::setSwipeEnabled(bool enabled)
{
    isSwipeUnlocked = enabled;
}

bool LeftPanel::eventFilter(QObject *watched, QEvent *event)
{
    if (!isSwipeUnlocked) return false;

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *me = static_cast<QMouseEvent*>(event);
        QPoint gp       = me->globalPosition().toPoint();
        QPoint ftl      = footerStack->mapToGlobal(QPoint(0, 0));
        QRect  fr(ftl, footerStack->size());
        dragStartPos    = fr.contains(gp) ? gp : QPoint(-1, -1);
    }
    else if (event->type() == QEvent::MouseButtonRelease) {
        if (dragStartPos.x() != -1) {
            QMouseEvent *me = static_cast<QMouseEvent*>(event);
            int dist        = me->globalPosition().toPoint().x() - dragStartPos.x();
            if (qAbs(dist) > 80) {
                footerStack->setCurrentIndex(dist > 0 ? 0 : 1);
                dragStartPos = QPoint(-1, -1);
                return true;
            }
        }
        dragStartPos = QPoint(-1, -1);
    }
    return QWidget::eventFilter(watched, event);
}