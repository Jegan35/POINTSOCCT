#include "RightPanel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QFormLayout>
#include <QResizeEvent>
#include <QMouseEvent>
#include <QFrame>
#include <QSlider>
#include <QDoubleSpinBox>
#include <QScrollArea>
#include <QHeaderView>
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QDialog>
#include <QDialogButtonBox>
#include <QValidator>
#include "OcctWidget.h"
#include <QTextEdit>

// ============================================================
//  OPTION LISTS  (mirrors QML frontend constants)
// ============================================================
static const QStringList INST_OPTIONS = {
    "Inst","MOVJ","MOVJ_dg","MOVL","MOVC","MVLEX_Deg","MVLEX_mm","Pallet_Matrix",
    "Num_of_row","Num_of_colm","pos_add_x","pos_add_y","pos_add_z","delay_ms",
    "go_to","loop","Start If","End If","Start-Con","End-Con","Wait",
    "DI-1","DI-2","DI-3","DI-4","DI-5","DI-6","DI-7","DI-8",
    "DI-9","DI-10","DI-11","DI-12","DI-13","DI-14","DI-15","DI-16",
    "DO-1","DO-2","DO-3","DO-4","DO-5","DO-6","DO-7","DO-8",
    "DO-9","DO-10","DO-11","DO-12","DO-13","DO-14","DO-15","DO-16",
    "AI-1","AI-2","AI-3","AI-4","AO-1","AO-2","AO-3","AO-4",
    "= Assign","== Equal","!= Not Eql","<",">","<=",">=","+","-","&","stop","Servo off"
};
static const QStringList DI_OPTIONS   = {"Di-1","D-1","D-2","D-3","D-4","D-5","D-6","D-7","D-8","D-9","D-10","D-11","D-12","D-13","D-14","D-15","D-16"};
static const QStringList DI2_OPTIONS  = {"Di-2","D-1","D-2","D-3","D-4","D-5","D-6","D-7","D-8","D-9","D-10","D-11","D-12","D-13","D-14","D-15","D-16"};
static const QStringList DIG_STATE    = {"DIG-S","High","Low"};
static const QStringList VAR1_OPTIONS = {"Vr_1","V-1","V-2","V-3","V-4","V-5","V-6","V-7","V-8","V-9","V-10","AI-1","AI-2","AI-3","AI-4","AO-1","AO-2","AO-3","AO-4"};
static const QStringList VAR2_OPTIONS = {"Vr_2","V-1","V-2","V-3","V-4","V-5","V-6","V-7","V-8","V-9","V-10","AI-1","AI-2","AI-3","AI-4","AO-1","AO-2","AO-3","AO-4"};
static const QStringList VAR_MONITOR  = {"V-1","V-2","V-3","V-4","V-5","V-6","V-7","V-8","V-9","V-10","AI-1","AI-2","AI-3","AI-4","AO-1","AO-2","AO-3","AO-4"};
static const QStringList DI_SIM_LIST  = {"DI","DI-1","DI-2","DI-3","DI-4","DI-5","DI-6","DI-7","DI-8","DI-9","DI-10","DI-11","DI-12","DI-13","DI-14","DI-15","DI-16"};
static const QStringList DO_SIM_LIST  = {"DO","DO-1","DO-2","DO-3","DO-4","DO-5","DO-6","DO-7","DO-8","DO-9","DO-10","DO-11","DO-12","DO-13","DO-14","DO-15","DO-16"};
static const QStringList SIM_STATE    = {"State","High","Low"};
static const QStringList TP_MODE_LIST = {"TP Mode","MOVJ","MOVL","ABSJ","MOVC"};
static const QStringList TP_NAME_CMD  = {"Tp name"};
static const QStringList PR_PARAM_LIST= {"Com","Delay","Go to","Loop","Speed","Ip Pg","Radius","AN ip"};

// ============================================================
//  STYLE CONSTANTS
// ============================================================
static const QString S_COMBO =
    "QComboBox { background:#1a1e2a; color:#e0e0e0; border:1px solid #2a2d35; border-radius:3px; "
    "padding:3px 6px; font-size:11px; font-weight:bold; min-height:26px; }"
    "QComboBox::drop-down { border:none; }"
    "QComboBox QAbstractItemView { background:#1a1e2a; color:#e0e0e0; "
    "selection-background-color:#00bcd4; selection-color:#000; }";

static const QString S_INPUT =
    "QLineEdit { background:#1a1e2a; color:#e0e0e0; border:1px solid #2a2d35; border-radius:3px; "
    "padding:3px 8px; font-size:11px; min-height:26px; }"
    "QLineEdit:focus { border-color:#00bcd4; }";

// Table cell styles — tight & readable
static const QString S_TBL_CELL =
    "QLabel { background:#0d1117; color:#cfd8dc; border:1px solid #1e2330; "
    "padding:2px 5px; font-size:11px; }";

static const QString S_TBL_HEAD =
    "QLabel { background:#0a0d14; color:#00bcd4; border:1px solid #2a2d35; "
    "padding:2px 5px; font-size:11px; font-weight:bold; }";

static const QString S_TBL_SECTION =
    "QLabel { background:#131720; color:#00bcd4; border:none; "
    "padding:1px 5px; font-size:10px; font-weight:bold; letter-spacing:1px; }";

static const QString S_TBL_VALUE =
    "QLabel { background:#0d1117; color:#80cbc4; border:1px solid #1e2330; "
    "padding:2px 5px; font-size:11px; font-family:monospace; }";

static const QString S_MENU_STYLE =
    "QMenu { background:#1a1e2a; color:#e0e0e0; border:1px solid #2a2d35; }"
    "QMenu::item { padding:8px 20px; }"
    "QMenu::item:selected { background:#00bcd4; color:#000; }";

// Button helpers
static QString sBtnColor(const QString &top, const QString &bot, const QString &edge) {
    return QString(
        "QPushButton { background:qlineargradient(y1:0,y2:1,stop:0 %1,stop:1 %2); "
        "color:white; font-weight:bold; border-radius:4px; border-bottom:3px solid %3; "
        "padding:0 6px; font-size:11px; min-height:28px; }"
        "QPushButton:pressed { margin-top:2px; border-bottom:1px solid %3; }"
    ).arg(top, bot, edge);
}
static const QString S_BTN_TEAL   = sBtnColor("#00bcd4","#0097a7","#006064");
static const QString S_BTN_BLUE   = sBtnColor("#1565C0","#0d47a1","#0a3060");
static const QString S_BTN_GREEN  = sBtnColor("#2E7D32","#1b5e20","#103a14");
static const QString S_BTN_PURPLE = sBtnColor("#6A1B9A","#4a148c","#2a0a50");
static const QString S_BTN_RED    = sBtnColor("#c62828","#b71c1c","#7f0000");
static const QString S_BTN_DARK   = sBtnColor("#37474f","#263238","#1a2327");
static const QString S_BTN_ORANGE = sBtnColor("#e65100","#bf360c","#8d2500");

// ============================================================
//  CONSTRUCTOR
// ============================================================
RightPanel::RightPanel(ClientBackend *backend, QWidget *parent)
    : QWidget(parent), m_backend(backend)
{
    m_uiThrottle.start();
    setupUI();

    if (m_backend) {
        connect(m_backend, &ClientBackend::telemetryChanged,
                this, &RightPanel::onTelemetryChanged);
        connect(m_backend, &ClientBackend::programDataChanged,
                this, &RightPanel::onProgramDataChanged);
        connect(m_backend, &ClientBackend::directoryDataChanged,
                this, &RightPanel::onDirectoryDataChanged);
        connect(m_backend, &ClientBackend::localStateChanged,
                this, &RightPanel::onLocalStateChanged);
        connect(m_backend, &ClientBackend::highlightedInstructionChanged,
                this, &RightPanel::onHighlightChanged);
    }

    qApp->installEventFilter(this);
}

// ============================================================
//  setupUI
// ============================================================
void RightPanel::setupUI()
{
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setContentsMargins(0,0,0,0);
    m_mainLayout->setSpacing(0);

    // ---- 1. HEADER  (stretch 1) ----
    header = new RightHeader(m_backend, this);
    header->setStyleSheet("background-color:#1E1E24; border-bottom:2px solid #3E3E42;");

    // ---- 2. CONTROL STACK  (stretch 4) ----
    controlStack = new QStackedWidget(this);
    controlStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    // --- Cartesian D-Pad page (index 0) ---
    QWidget *cartWidget = new QWidget(this);
    cartWidget->setStyleSheet("background-color:#151822;");
    QVBoxLayout *dpadLay = new QVBoxLayout(cartWidget);
    dpadLay->setContentsMargins(20,5,20,5);
    dpadLay->setSpacing(10);

    // Toggle pill
    QFrame *pillFrame = new QFrame(this);
    pillFrame->setStyleSheet("QFrame{background:#1E1E24;border:1px solid #3E3E42;border-radius:18px;}");
    pillFrame->setFixedSize(180,36);
    QHBoxLayout *pillLay = new QHBoxLayout(pillFrame);
    pillLay->setContentsMargins(2,2,2,2); pillLay->setSpacing(0);
    btnToggleXYZ    = new QPushButton("CART", pillFrame);
    btnToggleOrient = new QPushButton("JNT",  pillFrame);
    const QString XYZ_ACT  = "QPushButton{color:black;font-weight:bold;border:none;border-radius:14px;background:#00E5FF;}";
    const QString ORI_ACT  = "QPushButton{color:white;font-weight:bold;border:none;border-radius:14px;background:#22C55E;}";
    const QString INACT    = "QPushButton{color:#888;font-weight:bold;border:none;border-radius:14px;background:transparent;}";
    btnToggleXYZ->setStyleSheet(XYZ_ACT);
    btnToggleOrient->setStyleSheet(INACT);
    pillLay->addWidget(btnToggleXYZ); pillLay->addWidget(btnToggleOrient);
    QHBoxLayout *pillRow = new QHBoxLayout(); pillRow->addWidget(pillFrame); pillRow->addStretch();
    dpadLay->addLayout(pillRow);

    // D-pad buttons
    const QString B_RED  = "QPushButton{background:#EF4444;color:white;font-weight:bold;font-size:20px;border-radius:8px;border-bottom:5px solid #B91C1C;min-width:60px;min-height:60px;}QPushButton:pressed{margin-top:5px;border-bottom:0;}";
    const QString B_GRN  = "QPushButton{background:#4ADE80;color:white;font-weight:bold;font-size:20px;border-radius:8px;border-bottom:5px solid #16A34A;min-width:60px;min-height:60px;}QPushButton:pressed{margin-top:5px;border-bottom:0;}";
    const QString B_BLU  = "QPushButton{background:#0EA5E9;color:white;font-weight:bold;font-size:20px;border-radius:8px;border-bottom:5px solid #0369A1;min-width:60px;min-height:60px;}QPushButton:pressed{margin-top:5px;border-bottom:0;}";
    const QString B_HOME = "QPushButton{background:#0284C7;color:white;font-weight:bold;font-size:13px;border-radius:30px;border-bottom:5px solid #075985;min-width:60px;min-height:60px;max-width:60px;max-height:60px;}QPushButton:pressed{margin-top:5px;border-bottom:0;}";

    btnYPlus  = new QPushButton("Y+",this); btnYPlus->setStyleSheet(B_GRN);
    btnYMinus = new QPushButton("Y-",this); btnYMinus->setStyleSheet(B_GRN);
    btnXPlus  = new QPushButton("X+",this); btnXPlus->setStyleSheet(B_RED);
    btnXMinus = new QPushButton("X-",this); btnXMinus->setStyleSheet(B_RED);
    btnZPlus  = new QPushButton("Z+",this); btnZPlus->setStyleSheet(B_BLU);
    btnZMinus = new QPushButton("Z-",this); btnZMinus->setStyleSheet(B_BLU);
    QPushButton *btnHome = new QPushButton("HOME",this); btnHome->setStyleSheet(B_HOME);

    auto connectJog = [this](QPushButton *b){
        connect(b,&QPushButton::pressed, this,[this,b]{ if(m_backend) m_backend->handleButtonPress(b->text()); });
        connect(b,&QPushButton::released,this,[this,b]{ if(m_backend) m_backend->handleButtonRelease(b->text()); });
    };
    connectJog(btnXPlus); connectJog(btnXMinus);
    connectJog(btnYPlus); connectJog(btnYMinus);
    connectJog(btnZPlus); connectJog(btnZMinus);
    connect(btnHome,&QPushButton::clicked,this,[this]{ if(m_backend) m_backend->triggerHome(); });
    QGridLayout *jogGrid = new QGridLayout();
    jogGrid->setSpacing(10);
    jogGrid->setAlignment(Qt::AlignCenter);

    jogGrid->addWidget(btnYPlus,  0,1); jogGrid->addWidget(btnXMinus,1,0);
    jogGrid->addWidget(btnHome,   1,1); jogGrid->addWidget(btnXPlus, 1,2);
    jogGrid->addWidget(btnYMinus, 2,1); jogGrid->addWidget(btnZPlus, 0,3);
    jogGrid->addWidget(btnZMinus, 2,3);

    dpadLay->addLayout(jogGrid);
    dpadLay->addStretch(); // ✅ NEW: This physically stops the D-Pad from stretching vertically!

    controlStack->addWidget(cartWidget);       // index 0 – Cart D-pad
    controlStack->addWidget(buildSpeedPanel()); // index 1 – Speed config
    controlStack->addWidget(buildJointsPanel()); // index 2 – Joints

    // ---- 3. WORKSPACE TABS  (stretch 5) ----
    m_workspaceTabs = new QTabWidget(this);
    m_workspaceTabs->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_workspaceTabs->setStyleSheet(
        "QTabWidget::pane{border-top:3px solid #00E5FF;background:#151822;}"
        "QTabBar::tab{background:#1E1E24;color:#9CA3AF;padding:7px 13px;font-weight:bold;"
        "font-size:11px;text-transform:uppercase;border:none;}"
        "QTabBar::tab:selected{color:#00E5FF;border-bottom:3px solid #00E5FF;background:#151822;}"
        "QTabBar::tab:hover{color:#fff;}"
    );

    m_btnMax = new QPushButton("⛶ MAX", this);
    m_btnMax->setStyleSheet("color:#00E5FF;background:transparent;font-weight:bold;"
                            "font-size:13px;border:none;padding:0 14px;");
    m_btnMax->setCursor(Qt::PointingHandCursor);

    // ✅ FIX: Stops the MAX button from stealing keyboard focus,
    // which prevents random menus from popping open!
    m_btnMax->setFocusPolicy(Qt::NoFocus);

    m_workspaceTabs->setCornerWidget(m_btnMax, Qt::TopRightCorner);
    connect(m_btnMax, &QPushButton::clicked, this, &RightPanel::toggleMaximized);

    m_workspaceTabs->addTab(buildPrTableWidget(),    "PROGRAM FILE"); // 0
    m_workspaceTabs->addTab(buildIOModulesWidget(),  "I/O PANEL");    // 1
    m_workspaceTabs->addTab(buildTpTableWidget(),    "TP FILE");      // 2
    m_workspaceTabs->addTab(buildDataVarWidget(),    "DATA VAR");     // 3
    m_workspaceTabs->addTab(buildAxisLimitWidget(),  "AXIS LIMIT");   // 4

    // ✅ ADD THE NEW DXF TAB HERE (Index 5)
    m_workspaceTabs->addTab(buildDxfFileWidget(),    "DXF FILE");     // 5

    // ---- 4. INSTRUCTION TABLE  (hidden by default) ----
    m_instructionTableWidget = buildInstructionTableWidget();
    m_instructionTableWidget->hide();
    m_instructionTableWidget->setMinimumHeight(0);

    // ---- 5. CONTROL SWIPE STACK  (hidden by default) ----
    m_controlSwipeStack = new QStackedWidget(this);
    m_controlSwipeStack->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);
    m_controlSwipeStack->setMinimumHeight(0);
    m_controlSwipeStack->addWidget(buildTpCtrlWidget()); // 0 – TP CTRL
    m_controlSwipeStack->addWidget(buildPrCtrlWidget()); // 1 – PR CTRL
    m_controlSwipeStack->hide();

    // ---- ASSEMBLE MAIN LAYOUT ----
    m_mainLayout->addWidget(header, 0);                  // 0 = Take only minimum height
    m_mainLayout->addWidget(controlStack, 0);            // 0 = Do NOT stretch vertically
    m_mainLayout->addWidget(m_workspaceTabs, 1);         // 1 = Take ALL remaining screen space
    m_mainLayout->addWidget(m_instructionTableWidget, 0);
    m_mainLayout->addWidget(m_controlSwipeStack, 0);

    // ---- MENU TRAY ----
    tray = new MenuTray(m_backend, this);
    tray->hide();

    connect(header,&RightHeader::menuClicked,this,[this]{
        tray->toggleTray(this->width(), this->height());
    });
    connect(header,&RightHeader::swipeLockChanged,this,[this](bool u){
        emit swipeLockToggled(u);
    });

    connect(tray,&MenuTray::modeSelected,this,[this,XYZ_ACT,ORI_ACT,INACT](const QString &mode){
        if      (mode=="SPEED")     { header->updateStatusText("SETTINGS | GLOBAL SPEED",false);  controlStack->setCurrentIndex(1); }
        else if (mode=="JOG_CART")  { currentMovementMode="JOG";  header->updateStatusText("MANUAL | JOG : CARTESIAN",false);  controlStack->setCurrentIndex(0); }
        else if (mode=="JOG_JNT")   { currentMovementMode="JOG";  header->updateStatusText("MANUAL | JOG : JOINTS",false);    controlStack->setCurrentIndex(2); }
        else if (mode=="MOVE_CART") { currentMovementMode="MOVE"; header->updateStatusText("MANUAL | MOVE : CARTESIAN",false); controlStack->setCurrentIndex(0); }
        else if (mode=="MOVE_JNT")  { currentMovementMode="MOVE"; header->updateStatusText("MANUAL | MOVE : JOINTS",false);   controlStack->setCurrentIndex(2); }
        else if (mode=="AUTO")    { header->updateStatusText("AUTO | RUNNING",true);  }
        else                      { header->updateStatusText(mode+" | STANDBY",false); }
        tray->toggleTray(this->width(), this->height());
    });

    connect(btnToggleXYZ,&QPushButton::clicked,this,[=]{
        btnToggleXYZ->setStyleSheet(XYZ_ACT); btnToggleOrient->setStyleSheet(INACT);
        header->updateStatusText("MANUAL | "+currentMovementMode+" : CARTESIAN",false);
        btnXPlus->setText("X+"); btnXMinus->setText("X-");
        btnYPlus->setText("Y+"); btnYMinus->setText("Y-");
        btnZPlus->setText("Z+"); btnZMinus->setText("Z-");
    });
    connect(btnToggleOrient,&QPushButton::clicked,this,[=]{
        btnToggleOrient->setStyleSheet(ORI_ACT); btnToggleXYZ->setStyleSheet(INACT);
        header->updateStatusText("MANUAL | "+currentMovementMode+" : JOINT (RxRyRz)",false);
        btnXPlus->setText("Rx+"); btnXMinus->setText("Rx-");
        btnYPlus->setText("Ry+"); btnYMinus->setText("Ry-");
        btnZPlus->setText("Rz+"); btnZMinus->setText("Rz-");
    });
}

// ============================================================
//  toggleMaximized  –  MAX / MIN switch
//  In MAX mode: tabs(5) + staging(fixed 50px) + ctrl-swipe(2)
//  staging widget has setFixedHeight so stretch=0 is correct
// ============================================================
// ============================================================
//  toggleMaximized  –  MAX / MIN switch
// ============================================================
// ============================================================
//  toggleMaximized  –  MAX / MIN switch
// ============================================================
// ============================================================
//  toggleMaximized  –  MAX / MIN switch
// ============================================================
void RightPanel::toggleMaximized()
{
    // 🛑 FREEZE UI: Stops internal flickering
    this->setUpdatesEnabled(false);

    if (tray && !tray->isHidden()) {
        tray->hide();
    }

    static bool isMaximized = false;
    isMaximized = !isMaximized;

    QWidget* dxfView = this->findChild<QWidget*>("DxfViewArea");

    if (isMaximized) {
        m_btnMax->setText("🗗 MIN");
        header->hide();
        controlStack->hide();
        m_instructionTableWidget->show();
        m_controlSwipeStack->show();
        if (dxfView) dxfView->show();

        m_mainLayout->setStretch(0, 0);
        m_mainLayout->setStretch(1, 0);
        m_mainLayout->setStretch(2, 5);
        m_mainLayout->setStretch(3, 2);
        m_mainLayout->setStretch(4, 0);
    } else {
        m_btnMax->setText("⛶ MAX");
        header->show();
        controlStack->show();
        m_instructionTableWidget->hide();
        m_controlSwipeStack->hide();
        if (dxfView) dxfView->hide();

        m_mainLayout->setStretch(0, 0);
        m_mainLayout->setStretch(1, 0);
        m_mainLayout->setStretch(2, 1);
        m_mainLayout->setStretch(3, 0);
        m_mainLayout->setStretch(4, 0);
    }

    // 🟢 UNFREEZE UI
    this->setUpdatesEnabled(true);
    emit maximizedToggled(isMaximized);
}

// ============================================================
//  buildPrTableWidget  –  PROGRAM FILE tab
//  ✅ FIX: Generous fixed widths to force scrolling after 4 columns
// ============================================================
QWidget* RightPanel::buildPrTableWidget()
{
    QWidget *w = new QWidget();
    w->setStyleSheet("background:#0d1117;");
    QVBoxLayout *lay = new QVBoxLayout(w);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(0);

    m_prTable = new QTableWidget(0, 12, w);
    m_prTable->setHorizontalHeaderLabels({
        "S.No","Inst","Name","Value","Speed","Deg","Rad","Tool","Frame","Com","Dist","Time"
    });

    m_prTable->setStyleSheet(
        "QTableWidget {"
        "  background:#0d1117; color:#cfd8dc; border:none;"
        "  gridline-color:#1e2330; font-size:12px;"
        "  selection-background-color:#1565C0; selection-color:#fff;"
        "}"
        "QHeaderView::section {"
        "  background:#0a0d14; color:#00bcd4; border:none;"
        "  border-right:1px solid #1e2330; border-bottom:2px solid #00bcd4;"
        "  padding:4px 4px; font-weight:bold; font-size:11px;"
        "}"
        "QTableWidget::item { padding:3px 5px; border-bottom:1px solid #181e2c; }"
        "QTableWidget::item:selected { background:#1565C0; color:#fff; }"
        "QScrollBar:vertical   { background:#0a0d14; width:8px; border:none; }"
        "QScrollBar::handle:vertical   { background:#2a3040; border-radius:4px; min-height:18px; }"
        "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical { height:0; }"
        "QScrollBar:horizontal { background:#0a0d14; height:10px; border:none; }"
        "QScrollBar::handle:horizontal { background:#2a3040; border-radius:5px; min-width:20px; }"
        "QScrollBar::add-line:horizontal,QScrollBar::sub-line:horizontal { width:0; }"
        );

    // ✅ FIX: Strict widths matching the instruction table
    m_prTable->setColumnWidth(0,  50);   // S.No
    m_prTable->setColumnWidth(1,  140);  // Inst
    m_prTable->setColumnWidth(2,  160);  // Name
    m_prTable->setColumnWidth(3,  200);  // Value
    // --- Scrollbar activates around here ---
    m_prTable->setColumnWidth(4,  90);   // Speed
    m_prTable->setColumnWidth(5,  160);  // Deg
    m_prTable->setColumnWidth(6,  90);   // Rad
    m_prTable->setColumnWidth(7,  90);   // Tool
    m_prTable->setColumnWidth(8,  90);   // Frame
    m_prTable->setColumnWidth(9,  250);  // Com
    m_prTable->setColumnWidth(10, 90);   // Dist
    m_prTable->setColumnWidth(11, 90);   // Time

    // Disable stretching and strictly enforce our fixed widths
    m_prTable->horizontalHeader()->setStretchLastSection(false);
    m_prTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_prTable->verticalHeader()->hide();
    m_prTable->verticalHeader()->setDefaultSectionSize(26);

    // Force scrollbars
    m_prTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_prTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_prTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    m_prTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_prTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_prTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_prTable->setFocusPolicy(Qt::NoFocus);
    m_prTable->setAlternatingRowColors(false);
    m_prTable->setShowGrid(false);

    connect(m_prTable, &QTableWidget::clicked, this, [this](const QModelIndex &idx){
        if (m_backend) m_backend->selectProgramRow(idx.row());
    });

    lay->addWidget(m_prTable);
    return w;
}

// ============================================================
//  buildTpTableWidget  –  TP FILE tab
//  ✅ FIX: Generous fixed widths to force scrolling after 4 columns
// ============================================================
QWidget* RightPanel::buildTpTableWidget()
{
    QWidget *w = new QWidget();
    w->setStyleSheet("background:#0d1117;");
    QVBoxLayout *lay = new QVBoxLayout(w);
    lay->setContentsMargins(0,0,0,0);
    lay->setSpacing(0);

    m_tpTable = new QTableWidget(0, 6, w);
    m_tpTable->setHorizontalHeaderLabels({"S.No","Name","Value","Deg","Tool","Frame"});

    m_tpTable->setStyleSheet(
        "QTableWidget {"
        "  background:#0d1117; color:#cfd8dc; border:none;"
        "  gridline-color:#1e2330; font-size:12px;"
        "  selection-background-color:#4CAF50; selection-color:#000;"
        "}"
        "QHeaderView::section {"
        "  background:#0a0d14; color:#4CAF50; border:none;"
        "  border-right:1px solid #1e2330; border-bottom:2px solid #4CAF50;"
        "  padding:4px 4px; font-weight:bold; font-size:11px;"
        "}"
        "QTableWidget::item { padding:3px 5px; border-bottom:1px solid #181e2c; }"
        "QTableWidget::item:selected { background:#4CAF50; color:#000; }"
        "QScrollBar:vertical   { background:#0a0d14; width:8px; border:none; }"
        "QScrollBar::handle:vertical   { background:#2a3040; border-radius:4px; min-height:18px; }"
        "QScrollBar::add-line:vertical,QScrollBar::sub-line:vertical { height:0; }"
        "QScrollBar:horizontal { background:#0a0d14; height:10px; border:none; }"
        "QScrollBar::handle:horizontal { background:#2a3040; border-radius:5px; min-width:20px; }"
        "QScrollBar::add-line:horizontal,QScrollBar::sub-line:horizontal { width:0; }"
        );

    // ✅ FIX: Strict widths matching the instruction table
    m_tpTable->setColumnWidth(0,  50);   // S.No
    m_tpTable->setColumnWidth(1,  160);  // Name
    m_tpTable->setColumnWidth(2,  200);  // Value
    m_tpTable->setColumnWidth(3,  160);  // Deg
    // --- Scrollbar activates around here ---
    m_tpTable->setColumnWidth(4,  90);   // Tool
    m_tpTable->setColumnWidth(5,  90);   // Frame

    // Disable stretching and strictly enforce our fixed widths
    m_tpTable->horizontalHeader()->setStretchLastSection(false);
    m_tpTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Fixed);
    m_tpTable->verticalHeader()->hide();
    m_tpTable->verticalHeader()->setDefaultSectionSize(26);

    // Force scrollbars
    m_tpTable->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
    m_tpTable->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_tpTable->setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    m_tpTable->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_tpTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_tpTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_tpTable->setFocusPolicy(Qt::NoFocus);
    m_tpTable->setShowGrid(false);

    connect(m_tpTable, &QTableWidget::clicked, this, [this](const QModelIndex &idx){
        if (m_backend) m_backend->selectTpPoint(idx.row());
    });

    lay->addWidget(m_tpTable);
    return w;
}

// ============================================================
//  buildIOModulesWidget  –  I/O PANEL tab
// ============================================================
QWidget* RightPanel::buildIOModulesWidget()
{
    QWidget *w = new QWidget();
    w->setStyleSheet("background:#151822;");
    QVBoxLayout *vl = new QVBoxLayout(w);
    vl->setContentsMargins(10,10,10,10);
    vl->setSpacing(12);

    auto makeIoBox = [&](const QString &title, const QString &color, QLabel **leds) -> QWidget* {
        QWidget *box = new QWidget();
        box->setStyleSheet(QString("background:#1a1e2a;border:2px solid %1;border-radius:6px;").arg(color));
        QVBoxLayout *bl = new QVBoxLayout(box); bl->setContentsMargins(10,8,10,8); bl->setSpacing(6);

        QLabel *t = new QLabel(title);
        t->setStyleSheet(QString("color:%1;font-weight:bold;font-size:12px;background:transparent;border:none;").arg(color));
        t->setAlignment(Qt::AlignCenter); bl->addWidget(t);

        QHBoxLayout *ledRow = new QHBoxLayout(); ledRow->setSpacing(3);
        for (int i = 0; i < 16; i++) {
            QVBoxLayout *col = new QVBoxLayout(); col->setSpacing(2); col->setAlignment(Qt::AlignCenter);
            leds[i] = new QLabel(); leds[i]->setFixedSize(18,18);
            leds[i]->setStyleSheet("background:#2a2d35;border-radius:9px;border:1px solid #3a3d45;");
            QLabel *n = new QLabel(QString::number(i+1));
            n->setStyleSheet("color:#666;font-size:8px;background:transparent;border:none;");
            n->setAlignment(Qt::AlignCenter);
            col->addWidget(leds[i]); col->addWidget(n);
            ledRow->addLayout(col);
        }
        bl->addLayout(ledRow);
        return box;
    };

    vl->addWidget(makeIoBox("DIGITAL INPUTS (DI 1-16)",  "#4CAF50", m_diLeds));
    vl->addWidget(makeIoBox("DIGITAL OUTPUTS (DO 1-16)", "#039BE5", m_doLeds));
    vl->addStretch();
    return w;
}

// ============================================================
//  buildDataVarWidget  –  DATA VAR tab
//  Layout: label column | value column, neat pairs
// ============================================================
QWidget* RightPanel::buildDataVarWidget()
{
    QWidget *w = new QWidget();
    w->setStyleSheet("background:#0d1117;");
    QHBoxLayout *hl = new QHBoxLayout(w);
    hl->setContentsMargins(12,10,12,10);
    hl->setSpacing(12);

    // Section factory: returns a framed card with title + VBox for rows
    auto makeCard = [](const QString &title, const QString &accentColor) -> QPair<QWidget*,QVBoxLayout*> {
        QWidget *card = new QWidget();
        card->setStyleSheet(QString(
            "QWidget { background:#1a1e2a; border:1px solid %1; border-radius:5px; }"
        ).arg(accentColor));
        QVBoxLayout *vl = new QVBoxLayout(card);
        vl->setContentsMargins(0,0,0,0); vl->setSpacing(0);

        // Title bar
        QLabel *hdr = new QLabel(title);
        hdr->setStyleSheet(QString(
            "QLabel { background:%1; color:#000; font-weight:bold; font-size:11px;"
            " padding:4px 8px; border:none; border-radius:0; }"
        ).arg(accentColor));
        hdr->setAlignment(Qt::AlignCenter);
        vl->addWidget(hdr);

        QWidget *body = new QWidget();
        body->setStyleSheet("QWidget { background:transparent; border:none; }");
        QVBoxLayout *bl = new QVBoxLayout(body);
        bl->setContentsMargins(8,8,8,8); bl->setSpacing(6);
        vl->addWidget(body);
        return {card, bl};
    };

    // Row factory: label + widget side by side
    auto makeRow = [](QVBoxLayout *parent, const QString &label, QWidget *widget, const QString &labelColor="#9ca3af") {
        QHBoxLayout *row = new QHBoxLayout();
        row->setSpacing(6);
        QLabel *lbl = new QLabel(label);
        lbl->setStyleSheet(QString("QLabel{color:%1;font-size:11px;font-weight:bold;"
                                   "background:transparent;border:none;min-width:60px;}").arg(labelColor));
        row->addWidget(lbl);
        row->addWidget(widget, 1);
        parent->addLayout(row);
    };

    // --- Output Monitor ---
    auto [outCard, outLay] = makeCard("📺 OUTPUT MONITOR", "#00bcd4");
    m_varOutSel = new QComboBox(); m_varOutSel->addItems(VAR_MONITOR); m_varOutSel->setStyleSheet(S_COMBO);
    m_varOutVal = new QLabel("0");
    m_varOutVal->setStyleSheet(
        "QLabel{background:#0d1117;color:#00bcd4;font-weight:bold;font-size:18px;"
        "border:1px solid #00bcd4;padding:6px;text-align:center;border-radius:3px;}"
    );
    m_varOutVal->setAlignment(Qt::AlignCenter);
    makeRow(outLay, "Variable:", m_varOutSel);
    makeRow(outLay, "Value:", m_varOutVal);
    outLay->addStretch();
    connect(m_varOutSel,&QComboBox::currentTextChanged,this,[this](const QString &t){
        if(m_backend) m_backend->setVariableOutputSelector(t);
    });

    // --- Input Control ---
    auto [inCard, inLay] = makeCard("🎛 INPUT CONTROL", "#4CAF50");
    m_varInSel   = new QComboBox(); m_varInSel->addItems(VAR_MONITOR); m_varInSel->setStyleSheet(S_COMBO);
    m_varInInput = new QLineEdit(); m_varInInput->setPlaceholderText("Value"); m_varInInput->setStyleSheet(S_INPUT);
    makeRow(inLay, "Variable:", m_varInSel);
    makeRow(inLay, "Value:", m_varInInput);
    inLay->addStretch();
    connect(m_varInSel,&QComboBox::currentTextChanged,this,[this](const QString &t){
        if(m_backend) m_backend->setVariableInputSelector(t);
    });
    connect(m_varInInput,&QLineEdit::editingFinished,this,[this]{
        if(m_backend) m_backend->setVariableInputValue(m_varInInput->text());
    });

    // --- Instruction No. ---
    auto [instCard, instLay] = makeCard("🏷 INSTRUCTION NO.", "#FF9800");
    m_varInstNum = new QLineEdit(); m_varInstNum->setPlaceholderText("#"); m_varInstNum->setStyleSheet(S_INPUT);
    m_varInstNum->setValidator(new QIntValidator(1, 9999, this));
    makeRow(instLay, "Inst #:", m_varInstNum);
    instLay->addStretch();
    connect(m_varInstNum,&QLineEdit::editingFinished,this,[this]{
        if(m_backend) m_backend->setVariableInstructionNumber(m_varInstNum->text());
    });

    hl->addWidget(outCard);
    hl->addWidget(inCard);
    hl->addWidget(instCard);
    return w;
}

// ============================================================
//  buildAxisLimitWidget  –  AXIS LIMIT tab
//  Neat label|value pairs for each section
// ============================================================
QWidget* RightPanel::buildAxisLimitWidget()
{
    QWidget *w = new QWidget();
    w->setStyleSheet("background:#0d1117;");
    QHBoxLayout *hl = new QHBoxLayout(w);
    hl->setContentsMargins(10,10,10,10);
    hl->setSpacing(10);

    const QString cardSt  = "background:#1a1e2a;border:1px solid #2a2d35;border-radius:5px;";
    const QString headSt  = "color:#fff;font-weight:bold;font-size:11px;padding:4px 8px;"
                             "background:#263238;border:none;";
    const QString lblSt   = "color:#9ca3af;font-size:11px;background:transparent;border:none;min-width:65px;";
    const QString inpSt   = "QLineEdit{background:#0d1117;color:#e0e0e0;border:1px solid #2a2d35;"
                             "border-radius:3px;padding:2px 6px;font-size:11px;min-height:22px;}"
                             "QLineEdit:focus{border-color:#00bcd4;}";

    auto mkLbl = [&](const QString &t){ auto *l=new QLabel(t); l->setStyleSheet(lblSt); return l; };
    auto mkInp = [&](){
        auto *e=new QLineEdit(); e->setStyleSheet(inpSt); e->setFixedHeight(24);
        return e;
    };
    auto mkBtn = [&](const QString &t, const QString &st){
        auto *b=new QPushButton(t); b->setStyleSheet(st);
        b->setFixedHeight(24); return b;
    };
    auto mkRow = [&](QGridLayout *g, int row, const QString &lbl, QWidget *w1, QWidget *w2=nullptr){
        g->addWidget(mkLbl(lbl), row, 0);
        if(w2){
            g->addWidget(w1, row, 1); g->addWidget(w2, row, 2);
        } else {
            g->addWidget(w1, row, 1, 1, 2);
        }
    };

    // --- Digital Outputs ---
    QWidget *doCard = new QWidget(); doCard->setStyleSheet(cardSt);
    QVBoxLayout *doVl = new QVBoxLayout(doCard); doVl->setContentsMargins(0,0,0,0); doVl->setSpacing(0);
    QLabel *doH = new QLabel("🔌 Digital Outputs"); doH->setStyleSheet(headSt);
    doVl->addWidget(doH);
    QWidget *doBody = new QWidget(); QVBoxLayout *doBl = new QVBoxLayout(doBody);
    doBl->setContentsMargins(8,8,8,8); doBl->setSpacing(5);
    doVl->addWidget(doBody);
    QGridLayout *doGrid = new QGridLayout(); doGrid->setSpacing(5); doGrid->setColumnStretch(1,1);
    mkRow(doGrid, 0, "Digital Out:", mkInp());
    mkRow(doGrid, 1, "Analog 1:", mkInp());
    mkRow(doGrid, 2, "Analog 2:", mkInp());
    doBl->addLayout(doGrid); doBl->addStretch();
    hl->addWidget(doCard);

    // --- Digital Inputs ---
    QWidget *diCard = new QWidget(); diCard->setStyleSheet(cardSt);
    QVBoxLayout *diVl = new QVBoxLayout(diCard); diVl->setContentsMargins(0,0,0,0); diVl->setSpacing(0);
    QLabel *diH = new QLabel("🔘 Digital Inputs"); diH->setStyleSheet(headSt);
    diVl->addWidget(diH);
    QWidget *diBody = new QWidget(); QVBoxLayout *diBl = new QVBoxLayout(diBody);
    diBl->setContentsMargins(8,8,8,8); diBl->setSpacing(5);
    diVl->addWidget(diBody);
    QGridLayout *diGrid = new QGridLayout(); diGrid->setSpacing(5); diGrid->setColumnStretch(1,1);
    mkRow(diGrid, 0, "High 1 / Low 1:", mkBtn("High_1",S_BTN_GREEN), mkBtn("low_1",S_BTN_RED));
    mkRow(diGrid, 1, "High 2 / Low 2:", mkBtn("High_2",S_BTN_GREEN), mkBtn("low_2",S_BTN_RED));
    mkRow(diGrid, 2, "Test:", mkBtn("test_1",S_BTN_DARK));
    diBl->addLayout(diGrid); diBl->addStretch();
    hl->addWidget(diCard);

    // --- Simulation ---
    QWidget *simCard = new QWidget(); simCard->setStyleSheet(cardSt);
    QVBoxLayout *simVl = new QVBoxLayout(simCard); simVl->setContentsMargins(0,0,0,0); simVl->setSpacing(0);
    QLabel *simH = new QLabel("🖥 Simulation"); simH->setStyleSheet(headSt);
    simVl->addWidget(simH);
    QWidget *simBody = new QWidget(); QVBoxLayout *simBl = new QVBoxLayout(simBody);
    simBl->setContentsMargins(8,8,8,8); simBl->setSpacing(5);
    simVl->addWidget(simBody);

    m_simDiNum   = new QComboBox(); m_simDiNum->addItems(DI_SIM_LIST);  m_simDiNum->setStyleSheet(S_COMBO);
    m_simDiState = new QComboBox(); m_simDiState->addItems(SIM_STATE);   m_simDiState->setStyleSheet(S_COMBO);
    m_simDoNum   = new QComboBox(); m_simDoNum->addItems(DO_SIM_LIST);  m_simDoNum->setStyleSheet(S_COMBO);
    m_simDoState = new QComboBox(); m_simDoState->addItems(SIM_STATE);   m_simDoState->setStyleSheet(S_COMBO);

    QGridLayout *simGrid = new QGridLayout(); simGrid->setSpacing(5); simGrid->setColumnStretch(1,1);
    simGrid->addWidget(mkLbl("DI Sim:"), 0,0);
    simGrid->addWidget(m_simDiNum,  0,1); simGrid->addWidget(m_simDiState,0,2);
    simGrid->addWidget(mkLbl("DO Sim:"), 1,0);
    simGrid->addWidget(m_simDoNum,  1,1); simGrid->addWidget(m_simDoState,1,2);
    simGrid->addWidget(mkLbl("Remote:"), 2,0);
    simGrid->addWidget(mkBtn("rem_h",S_BTN_GREEN), 2,1);
    simGrid->addWidget(mkBtn("rem_l",S_BTN_RED),   2,2);
    simBl->addLayout(simGrid); simBl->addStretch();

    connect(m_simDiNum,  &QComboBox::currentTextChanged,this,[this](const QString &t){ if(m_backend) m_backend->setSimDiNumber(t); });
    connect(m_simDiState,&QComboBox::currentTextChanged,this,[this](const QString &t){ if(m_backend) m_backend->setSimDiState(t); });
    connect(m_simDoNum,  &QComboBox::currentTextChanged,this,[this](const QString &t){ if(m_backend) m_backend->setSimDoNumber(t); });
    connect(m_simDoState,&QComboBox::currentTextChanged,this,[this](const QString &t){ if(m_backend) m_backend->setSimDoState(t); });

    hl->addWidget(simCard, 2);
    return w;
}






// ============================================================
//  buildDxfFileWidget  –  DXF / STEP FILE tab
//  Layout: 50% Viewer Area | 50% Controls Area
// ============================================================
// ============================================================
//  buildDxfFileWidget  –  DXF / STEP FILE tab
// ============================================================
QWidget* RightPanel::buildDxfFileWidget()
{
    QWidget *w = new QWidget();
    w->setStyleSheet("background:#0d1117;");
    QHBoxLayout *dxfLayout = new QHBoxLayout(w);
    dxfLayout->setContentsMargins(15, 15, 15, 10);
    dxfLayout->setSpacing(20);

    // ----------------------------------------------------
    // LEFT 50% - VIEW AREA (Row 1) + ORIGIN DISPLAY (Row 2)
    // ----------------------------------------------------
    QWidget *dxfViewArea = new QWidget();
    dxfViewArea->setObjectName("DxfViewArea");
    dxfViewArea->setStyleSheet("background-color:#0a0d14; border:1px solid #1e2330; border-radius:5px;");

    // ✅ FIX 1: Prevent this container from expanding horizontally past 50%
    dxfViewArea->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *viewLayout = new QVBoxLayout(dxfViewArea);
    viewLayout->setContentsMargins(4, 4, 4, 4);
    viewLayout->setSpacing(8);

    // ROW 1: 3D Viewer
    m_dxfPreviewWidget = new OcctWidget(this);
    m_dxfPreviewWidget->setViewRole(OcctWidget::SideRole);

    // ✅ FIX 2: Muzzle the 3D widget's size demands
    m_dxfPreviewWidget->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    m_dxfPreviewWidget->setMinimumSize(50, 50);

    // ROW 2: Two Origin Readouts side-by-side
    QWidget *originContainer = new QWidget();
    originContainer->setStyleSheet("background:transparent; border:none;");
    // ✅ FIX 3: Lock the height of the origin readouts so they don't stretch
    originContainer->setFixedHeight(40);

    QHBoxLayout *originLay = new QHBoxLayout(originContainer);
    originLay->setContentsMargins(0, 0, 0, 0);
    originLay->setSpacing(10);

    // Box 1: Part Position Offset (Orange)
    QLabel *lblOrigin = new QLabel("Part Offset -> X: 0.000 | Y: -800.000 | Z: 0.000");
    lblOrigin->setStyleSheet("color:#F59E0B; font-weight:bold; font-size:11px; background:#141820; border:1px solid #3A4460; border-radius:4px; padding:6px;");
    lblOrigin->setAlignment(Qt::AlignCenter);

    // Box 2: 3D File / Extraction Origin (Cyan)
    QLabel *lblFileOrigin = new QLabel("3D File Origin -> X: 0.000 | Y: 0.000 | Z: 0.000");
    lblFileOrigin->setStyleSheet("color:#00E5FF; font-weight:bold; font-size:11px; background:#141820; border:1px solid #00838F; border-radius:4px; padding:6px;");
    lblFileOrigin->setAlignment(Qt::AlignCenter);

    originLay->addWidget(lblOrigin, 1);
    originLay->addWidget(lblFileOrigin, 1);

    // ✅ FIX 4: Explicit vertical stretches (1 for 3D, 0 for bottom bar)
    viewLayout->addWidget(m_dxfPreviewWidget, 1);
    viewLayout->addWidget(originContainer, 0);// Labels stay compact

    // ----------------------------------------------------
    // RIGHT 50% - CONTROLS AREA
    // ----------------------------------------------------
    QWidget *dxfControlArea = new QWidget();
    dxfControlArea->setStyleSheet("background:transparent; border:none;");
    QVBoxLayout *ctrlLayout = new QVBoxLayout(dxfControlArea);
    ctrlLayout->setContentsMargins(0, 0, 0, 0);
    ctrlLayout->setSpacing(12);

    // ✅ NEW: Selection Mode Dropdown (Face, Edge, Wire)
    QLabel *lblMode = new QLabel("SELECTION MODE:");
    lblMode->setStyleSheet("color:#00bcd4; font-weight:bold; font-size:11px; letter-spacing:1px;");

    QComboBox *cmbSelection = new QComboBox();
    cmbSelection->addItems({"Face (Surface)", "Edge (Line)", "Wire (Contour)"});

    // The Ultimate Dark-Mode QComboBox Style
    cmbSelection->setStyleSheet(
        /* 1. The Main Box (Pitch Black) */
        "QComboBox {"
        "  background-color: #050608; color: #FFFFFF;"
        "  border: 1px solid #2a2d35; padding: 8px 12px;"
        "  border-radius: 4px; font-weight: bold; font-size: 13px;"
        "}"
        /* 2. Hover Effect */
        "QComboBox:hover {"
        "  border: 1px solid #00bcd4;"
        "}"
        /* 3. The Arrow Button Area */
        "QComboBox::drop-down {"
        "  border-left: 1px solid #2a2d35; width: 26px;"
        "}"
        /* 4. The Popup Tray Box */
        "QComboBox QAbstractItemView {"
        "  background-color: #0a0d14;"
        "  color: #FFFFFF;"
        "  border: 1px solid #00bcd4;"  // Bright border so the tray pops out
        "  border-radius: 4px;"
        "  selection-background-color: #00bcd4;"
        "  selection-color: #000000;"   // Black text when hovered
        "  outline: none;"              // Removes the ugly dotted focus line
        "}"
        /* 5. The Items inside the Tray */
        "QComboBox QAbstractItemView::item {"
        "  min-height: 32px;"           // Gives the options plenty of breathing room
        "  padding-left: 8px;"
        "}"
        );

    // Wire the combo box to OCCT! (Index 0=Face(1), 1=Edge(2), 2=Wire(3))
    connect(cmbSelection, &QComboBox::currentIndexChanged, this, [this](int index){
        if (m_dxfPreviewWidget) m_dxfPreviewWidget->setSelectionMode(index + 1);
    });

    QLabel *lblDist = new QLabel("Distance (mm):");
    lblDist->setStyleSheet("color:#00bcd4; font-weight:bold; font-size:12px;");

    QLineEdit *txtDistance = new QLineEdit();
    txtDistance->setText("2.0");
    txtDistance->setStyleSheet(
        "QLineEdit { background:#1a1e2a; color:#ffffff; border:1px solid #2a2d35; "
        "padding:10px; border-radius:4px; font-size:14px; font-family:monospace; }"
        "QLineEdit:focus { border-color:#00bcd4; }"
        );

    m_btnGetPoints = new QPushButton("📍 GET POINTS");
    m_btnGetPoints->setEnabled(false);
    m_btnGetPoints->setStyleSheet("QPushButton { background-color:#2a3040; color:#64748b; font-weight:bold; padding:12px; border-radius:4px; border:none; font-size:13px; }");

    QPushButton *btnRunDxf = new QPushButton("▶ RUN");
    btnRunDxf->setStyleSheet("QPushButton { background-color:#10B981; color:#000000; font-weight:bold; padding:12px; border-radius:4px; border:none; font-size:13px; } QPushButton:hover { background-color:#059669; }");

    m_txtCoordinates = new QTextEdit();
    m_txtCoordinates->setReadOnly(true);
    m_txtCoordinates->setPlaceholderText("Extracted XYZ coordinates will appear here...");
    m_txtCoordinates->setStyleSheet("QTextEdit { background:#0a0d14; color:#00FF9D; border:1px solid #1e2330; border-radius:4px; padding:8px; font-family:'Consolas',monospace; font-size:12px; }");

    // ✅ FIX: Allow the text box to shrink so it NEVER pushes your buttons off the screen!
    m_txtCoordinates->setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Expanding);
    m_txtCoordinates->setMinimumHeight(40);
    // Add widgets to layout
    // Add widgets to layout
    ctrlLayout->addWidget(lblMode);
    ctrlLayout->addWidget(cmbSelection);
    ctrlLayout->addWidget(lblDist);
    ctrlLayout->addWidget(txtDistance);
    ctrlLayout->addWidget(m_btnGetPoints);
    ctrlLayout->addWidget(btnRunDxf);
    ctrlLayout->addWidget(m_txtCoordinates, 1);

    // ✅ FIX 5: Explicit horizontal stretches (Exactly 50/50)
    dxfLayout->addWidget(dxfViewArea, 1);
    dxfLayout->addWidget(dxfControlArea, 1);

    // ==========================================
    // ✅ WIRING FIX: Update new Cyan Origin box!
    // ==========================================
    // Notice how we pass `lblFileOrigin` into the lambda here now!
    connect(m_btnGetPoints, &QPushButton::clicked, this, [this, txtDistance, lblFileOrigin]() {
        double dist = txtDistance->text().toDouble();
        if (dist <= 0.001) dist = 2.0;

        m_dxfPreviewWidget->processCurrentSelection(dist);

        // ✅ Update the NEW Cyan label with the extraction origin
        lblFileOrigin->setText("3D File Origin -> " + m_dxfPreviewWidget->getOriginText());
    });

    // Wire coordinates output
    // Wire coordinates output
    connect(m_dxfPreviewWidget, &OcctWidget::coordinatesExtracted, this, [this](const QString &data) {
        m_txtCoordinates->setPlainText(data);
    });

    // ✅ ADD THIS BLOCK: Enables the GET POINTS button when you click a line/face
    connect(m_dxfPreviewWidget, &OcctWidget::selectionChanged, this, [this](bool hasSelection) {
        this->setGetPointsEnabled(hasSelection);
    });

    return w;
}




// ============================================================
//  buildInstructionTableWidget  –  staging row
//  ✅ FIX: Strict fixed column widths to force exactly 5 visible
// ============================================================
QWidget* RightPanel::buildInstructionTableWidget()
{
    QWidget *w = new QWidget();
    w->setStyleSheet("background:#0a0d14; border-top:2px solid #00bcd4;");
    QVBoxLayout *vl = new QVBoxLayout(w);
    vl->setContentsMargins(0,0,0,0);
    vl->setSpacing(0);

    // Section header bar
    QWidget *bar = new QWidget(); bar->setStyleSheet("background:#1a1e2a;");
    QHBoxLayout *bl = new QHBoxLayout(bar); bl->setContentsMargins(8,2,8,2);
    QLabel *barLbl = new QLabel("▶  STAGING INSTRUCTION");
    barLbl->setStyleSheet("color:#00bcd4;font-weight:bold;font-size:11px;"
                          "background:transparent;letter-spacing:1px;");
    bl->addWidget(barLbl); bl->addStretch();
    vl->addWidget(bar);

    // ---- Create Custom Scroll Area for Staging Data ----
    QScrollArea *scrollArea = new QScrollArea(w);
    scrollArea->setWidgetResizable(true); // Content expands
    scrollArea->setFrameShape(QFrame::NoFrame);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOn); // Always on
    scrollArea->setStyleSheet(
        "QScrollArea { background: #0d1117; border: none; }"
        "QScrollBar:horizontal { background:#0a0d14; height:10px; border:none; }"
        "QScrollBar::handle:horizontal { background:#2a3040; border-radius:5px; min-width:20px; }"
        "QScrollBar::add-line:horizontal, QScrollBar::sub-line:horizontal { width:0px; }"
        );

    QWidget *tableWrap = new QWidget();
    tableWrap->setStyleSheet("background:#0d1117;");
    QVBoxLayout *tw = new QVBoxLayout(tableWrap);
    tw->setContentsMargins(4,4,4,4); tw->setSpacing(2);

    // ✅ FIX: Changed to setFixedWidth so they never squish or stretch automatically
    auto mkH = [&](const QString &t, int fixedWidth) -> QLabel* {
        auto *l = new QLabel(t); l->setStyleSheet(S_TBL_HEAD);
        l->setAlignment(Qt::AlignCenter);
        l->setFixedWidth(fixedWidth);
        return l;
    };
    auto mkV = [&](int fixedWidth) -> QLabel* {
        auto *l = new QLabel("--"); l->setStyleSheet(S_TBL_VALUE);
        l->setAlignment(Qt::AlignCenter);
        l->setFixedWidth(fixedWidth);
        return l;
    };

    // --- Header row ---
    QWidget *hdrRow = new QWidget(); hdrRow->setStyleSheet("background:#0a0d14;");
    QHBoxLayout *hl = new QHBoxLayout(hdrRow); hl->setContentsMargins(0,0,0,0); hl->setSpacing(1);

    // ✅ Generous widths so exactly the first 5 fill the panel
    hl->addWidget(mkH("S.No", 50));         // 1
    hl->addWidget(mkH("Instruction", 140)); // 2
    hl->addWidget(mkH("Name 1", 160));      // 3
    hl->addWidget(mkH("Value 1", 200));     // 4
    hl->addWidget(mkH("Deg 1", 160));       // 5
    // -- Scrollbar activates around here --
    hl->addWidget(mkH("Name 2", 160));      // 6
    hl->addWidget(mkH("Value 2", 200));     // 7
    hl->addWidget(mkH("Deg 2", 160));       // 8
    hl->addWidget(mkH("Speed", 90));        // 9
    hl->addWidget(mkH("Radius", 90));       // 10
    hl->addWidget(mkH("Frame", 90));        // 11
    hl->addWidget(mkH("Tool", 90));         // 12
    hl->addWidget(mkH("Comment", 250));     // 13
    hl->addStretch(); // Absorbs any extra screen space
    tw->addWidget(hdrRow);

    // --- Data row ---
    QWidget *dataRow = new QWidget(); dataRow->setStyleSheet("background:#1a1e2a;");
    QHBoxLayout *dl = new QHBoxLayout(dataRow); dl->setContentsMargins(0,0,0,0); dl->setSpacing(1);

    QLabel *sno = mkV(50); sno->setText("1");
    m_stgInst    = mkV(140); m_stgName1 = mkV(160); m_stgVal1 = mkV(200);
    m_stgDeg1    = mkV(160); m_stgName2 = mkV(160); m_stgVal2 = mkV(200);
    m_stgDeg2    = mkV(160); m_stgSpeed = mkV(90);
    QLabel *radius = mkV(90);
    QLabel *frame  = mkV(90);
    QLabel *tool   = mkV(90);
    m_stgComment = mkV(250);

    dl->addWidget(sno); dl->addWidget(m_stgInst);
    dl->addWidget(m_stgName1); dl->addWidget(m_stgVal1);
    dl->addWidget(m_stgDeg1);  dl->addWidget(m_stgName2);
    dl->addWidget(m_stgVal2);  dl->addWidget(m_stgDeg2);
    dl->addWidget(m_stgSpeed); dl->addWidget(radius);
    dl->addWidget(frame);      dl->addWidget(tool);
    dl->addWidget(m_stgComment);
    dl->addStretch(); // Absorbs any extra screen space
    tw->addWidget(dataRow);

    scrollArea->setWidget(tableWrap);
    vl->addWidget(scrollArea);

    w->setMinimumHeight(100);
    w->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Expanding);

    return w;
}

// ============================================================
//  buildTpCtrlWidget  –  TP CTRL (index 0 in swipe stack)
//  ✅ FIX: 3-Row Layout with evenly distributed space
// ============================================================
QWidget* RightPanel::buildTpCtrlWidget()
{
    QWidget *w = new QWidget();
    w->setStyleSheet(
        "QWidget#TpCtrl { background:#151822;"
        "  border-top:2px solid #4CAF50; border-left:3px solid #4CAF50; }"
        );
    w->setObjectName("TpCtrl");

    QVBoxLayout *vl = new QVBoxLayout(w);
    vl->setContentsMargins(4,4,4,4);
    vl->setSpacing(4);

    // ── ROW 1: Toggle Tabs ──
    QWidget *togBar = new QWidget();
    togBar->setStyleSheet("background:#0d1117;border-radius:4px;");
    QHBoxLayout *tl = new QHBoxLayout(togBar); tl->setContentsMargins(4,2,4,2); tl->setSpacing(4);

    QPushButton *btnTpAct = new QPushButton("✏ TP EDIT");
    btnTpAct->setStyleSheet("QPushButton{background:#4CAF50;color:#000;font-weight:bold;border-radius:4px;padding:4px 14px;font-size:11px;border:none;}");

    QPushButton *btnPrSwitch = new QPushButton("✏ PR EDIT");
    btnPrSwitch->setStyleSheet("QPushButton{background:transparent;color:#888;font-weight:bold;border-radius:4px;padding:4px 14px;font-size:11px;border:none;}QPushButton:hover{color:#ccc;}");
    connect(btnPrSwitch, &QPushButton::clicked, this, [this]{ m_controlSwipeStack->setCurrentIndex(1); });

    tl->addWidget(btnTpAct); tl->addWidget(btnPrSwitch); tl->addStretch();
    vl->addWidget(togBar); // stretch 1

    // ── ROW 2: TP Action Buttons ──
    QHBoxLayout *rowA = new QHBoxLayout(); rowA->setSpacing(4);

    m_btnTpModeLabel = new QPushButton("⚙ TP Mode", w);
    m_btnTpModeLabel->setStyleSheet(S_BTN_BLUE);
    QMenu *menu = new QMenu(m_btnTpModeLabel);
    menu->setStyleSheet(S_MENU_STYLE);
    for (const QString &mode : TP_MODE_LIST) {
        const QString cmd = (mode == "TP Mode") ? "Tp" : mode;
        QAction *a = new QAction(mode, menu);
        connect(a, &QAction::triggered, this, [this, mode, cmd]{
            m_tpModeDisplay = mode;
            if (m_btnTpModeLabel) m_btnTpModeLabel->setText("⚙ " + mode);
            if (m_backend)        m_backend->setTpRunMode(cmd);
        });
        menu->addAction(a);
    }
    m_btnTpModeLabel->setMenu(menu);
    rowA->addWidget(m_btnTpModeLabel, 2);

    QPushButton *btnInsTP = new QPushButton("➕ Insert TP", w);
    btnInsTP->setStyleSheet(S_BTN_PURPLE);
    connect(btnInsTP, &QPushButton::clicked, this, [this]{ if (m_backend) m_backend->insertTpPoint(); });
    rowA->addWidget(btnInsTP, 2);

    QPushButton *btnModTP = new QPushButton("📝 Modify TP", w);
    btnModTP->setStyleSheet(S_BTN_ORANGE);
    connect(btnModTP, &QPushButton::clicked, this, [this]{
        if (!m_tpTable || !m_backend) return;
        int r = m_tpTable->currentRow();
        if (r >= 0) showModifyTpDialog(r);
    });
    rowA->addWidget(btnModTP, 2);

    QPushButton *btnDelTP = new QPushButton("🗑 Delete TP", w);
    btnDelTP->setStyleSheet(S_BTN_RED);
    connect(btnDelTP, &QPushButton::clicked, this, [this]{
        if (m_backend && m_tpTable) m_backend->deleteTpPoint(m_tpTable->currentRow());
    });
    rowA->addWidget(btnDelTP, 2);

    QPushButton *btnRunTp = new QPushButton("▶ Run TP", w);
    btnRunTp->setStyleSheet(S_BTN_GREEN);
    connect(btnRunTp, &QPushButton::clicked, this, [this]{ if (m_backend) m_backend->runTpPoint(); });
    rowA->addWidget(btnRunTp, 2);

    vl->addLayout(rowA); // stretch 1

    // ── ROW 3: Param Setter & Status ──
    // ── ROW 3: Param Setter & Status ──
    QHBoxLayout *rowB = new QHBoxLayout();
    rowB->setSpacing(4);

    QFrame *tpApplyFr = new QFrame(w);
    tpApplyFr->setStyleSheet("QFrame { background:#0a0d14; border:1px solid #2a2d35; border-radius:4px; }");
    QHBoxLayout *fl = new QHBoxLayout(tpApplyFr); fl->setContentsMargins(0,0,0,0); fl->setSpacing(0);

    m_tpParamSel = new QComboBox(w); m_tpParamSel->addItems({"Ip Pg", "Tp name", "Com"});
    m_tpParamSel->setStyleSheet(S_COMBO); m_tpParamSel->setFixedWidth(72);
    fl->addWidget(m_tpParamSel);

    QFrame *div = new QFrame(w); div->setFrameShape(QFrame::VLine);
    div->setStyleSheet("background:#2a2d35; max-width:1px; border:none;");
    fl->addWidget(div);

    m_tpParamVal = new QLineEdit(w); m_tpParamVal->setPlaceholderText("Value"); m_tpParamVal->setStyleSheet(S_INPUT);
    fl->addWidget(m_tpParamVal, 1);

    QPushButton *applyTp = new QPushButton("APPLY", w);
    applyTp->setStyleSheet(S_BTN_TEAL); applyTp->setFixedWidth(58);
    connect(applyTp, &QPushButton::clicked, this, [this]{
        if (!m_backend || !m_tpParamSel || !m_tpParamVal) return;
        const QString p = m_tpParamSel->currentText(), v = m_tpParamVal->text();
        if (v.isEmpty()) return;
        if      (p == "Ip Pg")   m_backend->setProgramInput(v);
        else if (p == "Tp name") m_backend->setTpPointName(v);
        else if (p == "Com")     m_backend->setProgramComment(v);
        m_tpParamVal->clear();
    });
    fl->addWidget(applyTp);
    rowB->addWidget(tpApplyFr, 4);

    QPushButton *btnCalc = new QPushButton("🧮 Calc Traj", w);
    btnCalc->setStyleSheet(S_BTN_TEAL);
    connect(btnCalc, &QPushButton::clicked, this, [this]{ if (m_backend) m_backend->calculateTrajectory(); });
    rowB->addWidget(btnCalc, 2);

    // ✅ Floating "MODE TP" widget completely removed!
    // ✅ Replaced with empty stretch space to keep buttons aligned
    rowB->addStretch(2);

    vl->addLayout(rowB); // stretch 1

    return w;
}

// ============================================================
//  buildPrCtrlWidget  –  PR CTRL (index 1 in swipe stack)
//  ✅ FIX: 3-Row Layout with evenly distributed space
// ============================================================
QWidget* RightPanel::buildPrCtrlWidget()
{
    QWidget *w = new QWidget();
    w->setStyleSheet(
        "QWidget#PrCtrl { background:#151822;"
        "  border-top:2px solid #00bcd4; border-left:3px solid #00bcd4; }"
        );
    w->setObjectName("PrCtrl");

    QVBoxLayout *vl = new QVBoxLayout(w);
    vl->setContentsMargins(4,4,4,4);
    vl->setSpacing(4);

    // ── ROW 1: Toggle Tabs ──
    QWidget *togBar = new QWidget();
    togBar->setStyleSheet("background:#0d1117;border-radius:4px;");
    QHBoxLayout *tl = new QHBoxLayout(togBar); tl->setContentsMargins(4,2,4,2); tl->setSpacing(4);

    QPushButton *btnTpSwitch = new QPushButton("✏ TP EDIT");
    btnTpSwitch->setStyleSheet("QPushButton{background:transparent;color:#888;font-weight:bold;border-radius:4px;padding:4px 14px;font-size:11px;border:none;}QPushButton:hover{color:#ccc;}");
    connect(btnTpSwitch, &QPushButton::clicked, this, [this]{ m_controlSwipeStack->setCurrentIndex(0); });

    QPushButton *btnPrAct = new QPushButton("✏ PR EDIT");
    btnPrAct->setStyleSheet("QPushButton{background:#00bcd4;color:#000;font-weight:bold;border-radius:4px;padding:4px 14px;font-size:11px;border:none;}");

    tl->addWidget(btnTpSwitch); tl->addWidget(btnPrAct); tl->addStretch();
    vl->addWidget(togBar); // stretch 1

    // ── ROW 2: Selectors ──
    QHBoxLayout *rowA = new QHBoxLayout(); rowA->setSpacing(4);

    m_instSel = new QComboBox(w); m_instSel->addItems(INST_OPTIONS); m_instSel->setStyleSheet(S_COMBO);
    connect(m_instSel, &QComboBox::currentTextChanged, this, [this](const QString &t){ if (m_backend) m_backend->setInstructionType(t); });
    rowA->addWidget(m_instSel, 2);

    m_di1Sel = new QComboBox(w); m_di1Sel->addItems(DI_OPTIONS); m_di1Sel->setStyleSheet(S_COMBO);
    connect(m_di1Sel, &QComboBox::currentTextChanged, this, [this](const QString &t){ if (m_backend) m_backend->setDigi1(t); });
    rowA->addWidget(m_di1Sel, 1);

    m_di2Sel = new QComboBox(w); m_di2Sel->addItems(DI2_OPTIONS); m_di2Sel->setStyleSheet(S_COMBO);
    connect(m_di2Sel, &QComboBox::currentTextChanged, this, [this](const QString &t){ if (m_backend) m_backend->setDigi2(t); });
    rowA->addWidget(m_di2Sel, 1);

    QPushButton *btnHL = new QPushButton("# H/L", w); btnHL->setStyleSheet(S_BTN_DARK);
    connect(btnHL, &QPushButton::clicked, this, [this]{ if (m_backend) m_backend->confirmHighLow(); });
    rowA->addWidget(btnHL, 1);

    m_hlSel = new QComboBox(w); m_hlSel->addItems(DIG_STATE); m_hlSel->setStyleSheet(S_COMBO);
    connect(m_hlSel, &QComboBox::currentTextChanged, this, [this](const QString &t){ if (m_backend) m_backend->setHighLow(t); });
    rowA->addWidget(m_hlSel, 1);

    m_var1Sel = new QComboBox(w); m_var1Sel->addItems(VAR1_OPTIONS); m_var1Sel->setStyleSheet(S_COMBO);
    connect(m_var1Sel, &QComboBox::currentTextChanged, this, [this](const QString &t){ if (m_backend) m_backend->setVariable1(t); });
    rowA->addWidget(m_var1Sel, 1);

    m_var2Sel = new QComboBox(w); m_var2Sel->addItems(VAR2_OPTIONS); m_var2Sel->setStyleSheet(S_COMBO);
    connect(m_var2Sel, &QComboBox::currentTextChanged, this, [this](const QString &t){ if (m_backend) m_backend->setVariable2(t); });
    rowA->addWidget(m_var2Sel, 1);

    vl->addLayout(rowA); // stretch 1

    // ── ROW 3: Actions & Params ──
    QHBoxLayout *rowB = new QHBoxLayout(); rowB->setSpacing(4);

    QFrame *prApplyFr = new QFrame(w);
    prApplyFr->setStyleSheet("QFrame { background:#0a0d14; border:1px solid #2a2d35; border-radius:4px; }");
    QHBoxLayout *fl = new QHBoxLayout(prApplyFr); fl->setContentsMargins(0,0,0,0); fl->setSpacing(0);

    m_prParamSel = new QComboBox(w); m_prParamSel->addItems(PR_PARAM_LIST); m_prParamSel->setStyleSheet(S_COMBO);
    m_prParamSel->setFixedWidth(72);
    fl->addWidget(m_prParamSel);

    QFrame *div = new QFrame(w); div->setFrameShape(QFrame::VLine); div->setStyleSheet("background:#2a2d35; max-width:1px; border:none;");
    fl->addWidget(div);

    m_prParamVal = new QLineEdit(w); m_prParamVal->setPlaceholderText("Value"); m_prParamVal->setStyleSheet(S_INPUT);
    fl->addWidget(m_prParamVal, 1);

    QPushButton *applyPr = new QPushButton("APPLY", w);
    applyPr->setStyleSheet(S_BTN_TEAL); applyPr->setFixedWidth(58);
    connect(applyPr, &QPushButton::clicked, this, [this]{
        if (!m_backend || !m_prParamSel || !m_prParamVal) return;
        const QString p = m_prParamSel->currentText(), v = m_prParamVal->text();
        if (v.isEmpty()) return;
        if      (p == "Com")    m_backend->setProgramComment(v);
        else if (p == "Delay")  m_backend->setDelay(v);
        else if (p == "Go to")  m_backend->setGoToProgram(v);
        else if (p == "Loop")   m_backend->setLoop(v);
        else if (p == "Speed")  m_backend->setProgramSpeed(v);
        else if (p == "Ip Pg")  m_backend->setProgramInput(v);
        else if (p == "Radius") m_backend->sendCmd("SET_RADIUS", v);
        else if (p == "AN ip")  m_backend->sendCmd("SET_AN_IP", v);
        m_prParamVal->clear();
    });
    fl->addWidget(applyPr);
    rowB->addWidget(prApplyFr, 3);

    QPushButton *btnInsInst = new QPushButton("➕ Inst", w); btnInsInst->setStyleSheet(S_BTN_PURPLE);
    connect(btnInsInst, &QPushButton::clicked, this, [this]{ if (m_backend) m_backend->insertProgramInstructionAt(0); });
    rowB->addWidget(btnInsInst, 1);

    QPushButton *btnModInst = new QPushButton("📝 Modify", w); btnModInst->setStyleSheet(S_BTN_ORANGE);
    connect(btnModInst, &QPushButton::clicked, this, [this]{
        if (!m_prTable || !m_backend) return;
        int r = m_prTable->currentRow();
        if (r >= 0) showModifyPrDialog(r);
    });
    rowB->addWidget(btnModInst, 1);

    QPushButton *btnDelInst = new QPushButton("🗑 Delete", w); btnDelInst->setStyleSheet(S_BTN_RED);
    connect(btnDelInst, &QPushButton::clicked, this, [this]{ if (m_backend) m_backend->deleteProgramInstruction(); });
    rowB->addWidget(btnDelInst, 1);

    QPushButton *btnRunInst = new QPushButton("▶ Run", w); btnRunInst->setStyleSheet(S_BTN_GREEN);
    connect(btnRunInst, &QPushButton::clicked, this, [this]{ if (m_backend) m_backend->runProgram(); });
    rowB->addWidget(btnRunInst, 1);

    QWidget *opPgW = new QWidget(w);
    opPgW->setStyleSheet("background:#0a0d14; border:1px solid #2a2d35; border-radius:4px;");
    QHBoxLayout *opl = new QHBoxLayout(opPgW); opl->setContentsMargins(0,0,0,0); opl->setSpacing(0);
    QLabel *opLbl = new QLabel(" OP PG ");
    opLbl->setStyleSheet("QLabel { color:#9ca3af; font-weight:bold; font-size:9px; background:#1a1e2a; border-right:1px solid #2a2d35; padding:0 4px; border:none; }");
    m_lblOpPg = new QLabel("0");
    m_lblOpPg->setStyleSheet("QLabel { color:#00bcd4; font-weight:bold; font-size:13px; background:#0d1117; padding:0 8px; min-width:30px; border:none; }");
    m_lblOpPg->setAlignment(Qt::AlignCenter);
    opl->addWidget(opLbl); opl->addWidget(m_lblOpPg, 1);
    rowB->addWidget(opPgW, 1);

    vl->addLayout(rowB); // stretch 1

    return w;
}

// ============================================================
//  showModifyTpDialog  –  replaces QML modifyTpPopup
// ============================================================
void RightPanel::showModifyTpDialog(int row)
{
    if (!m_tpTable) return;

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("Modify TP Point");
    dlg->setStyleSheet(
        "QDialog{background:#1e1e2e;}"
        "QLabel{color:#B0BEC5;font-size:12px;background:transparent;}"
        "QLineEdit{background:#0d1117;color:#e0e0e0;border:1px solid #9C27B0;"
        "border-radius:3px;padding:4px 8px;font-size:12px;}"
        "QLineEdit:focus{border-color:#CE93D8;}"
        "QTabWidget::pane{border:1px solid #9C27B0;background:#151822;}"
        "QTabBar::tab{background:#1a1e2a;color:#888;padding:6px 16px;font-weight:bold;border:none;}"
        "QTabBar::tab:selected{background:#9C27B0;color:#fff;}"
        "QPushButton{border-radius:4px;padding:8px 20px;font-weight:bold;font-size:12px;}"
    );
    dlg->setMinimumWidth(380);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *vl = new QVBoxLayout(dlg); vl->setSpacing(12);

    QLabel *title = new QLabel("  MODIFY TP POINT");
    title->setStyleSheet("color:#CE93D8;font-weight:bold;font-size:14px;"
                          "background:#12122a;padding:8px;border-bottom:2px solid #9C27B0;");
    vl->addWidget(title);

    // Name field
    QHBoxLayout *nameRow = new QHBoxLayout();
    nameRow->addWidget(new QLabel("Name:"));
    QLineEdit *edName = new QLineEdit();
    edName->setText(m_tpTable->item(row,1) ? m_tpTable->item(row,1)->text() : "");
    nameRow->addWidget(edName,1);
    vl->addLayout(nameRow);

    // Mode tabs: Cartesian | Joints
    QTabWidget *tabs = new QTabWidget();

    // Cartesian tab
    QWidget *cartTab = new QWidget(); cartTab->setStyleSheet("background:#151822;");
    QGridLayout *cg = new QGridLayout(cartTab); cg->setSpacing(8);
    QLineEdit *edX=new QLineEdit(); edX->setPlaceholderText("X (mm)");
    QLineEdit *edY=new QLineEdit(); edY->setPlaceholderText("Y (mm)");
    QLineEdit *edZ=new QLineEdit(); edZ->setPlaceholderText("Z (mm)");
    QLineEdit *edA=new QLineEdit(); edA->setPlaceholderText("A (deg)");
    QLineEdit *edB=new QLineEdit(); edB->setPlaceholderText("B (deg)");
    QLineEdit *edC=new QLineEdit(); edC->setPlaceholderText("C (deg)");
    auto addDV = [](QLineEdit *e){ e->setValidator(new QDoubleValidator()); };
    addDV(edX); addDV(edY); addDV(edZ); addDV(edA); addDV(edB); addDV(edC);
    cg->addWidget(new QLabel("X:"),0,0); cg->addWidget(edX,0,1);
    cg->addWidget(new QLabel("Y:"),0,2); cg->addWidget(edY,0,3);
    cg->addWidget(new QLabel("Z:"),1,0); cg->addWidget(edZ,1,1);
    cg->addWidget(new QLabel("A:"),1,2); cg->addWidget(edA,1,3);
    cg->addWidget(new QLabel("B:"),2,0); cg->addWidget(edB,2,1);
    cg->addWidget(new QLabel("C:"),2,2); cg->addWidget(edC,2,3);
    tabs->addTab(cartTab, "Cartesian");

    // Joints tab
    QWidget *jntTab = new QWidget(); jntTab->setStyleSheet("background:#151822;");
    QGridLayout *jg = new QGridLayout(jntTab); jg->setSpacing(8);
    QLineEdit *edJ1=new QLineEdit(); edJ1->setPlaceholderText("J1 (deg)");
    QLineEdit *edJ2=new QLineEdit(); edJ2->setPlaceholderText("J2 (deg)");
    QLineEdit *edJ3=new QLineEdit(); edJ3->setPlaceholderText("J3 (deg)");
    QLineEdit *edJ4=new QLineEdit(); edJ4->setPlaceholderText("J4 (deg)");
    QLineEdit *edJ5=new QLineEdit(); edJ5->setPlaceholderText("J5 (deg)");
    QLineEdit *edJ6=new QLineEdit(); edJ6->setPlaceholderText("J6 (deg)");
    addDV(edJ1); addDV(edJ2); addDV(edJ3); addDV(edJ4); addDV(edJ5); addDV(edJ6);
    jg->addWidget(new QLabel("J1:"),0,0); jg->addWidget(edJ1,0,1);
    jg->addWidget(new QLabel("J2:"),0,2); jg->addWidget(edJ2,0,3);
    jg->addWidget(new QLabel("J3:"),1,0); jg->addWidget(edJ3,1,1);
    jg->addWidget(new QLabel("J4:"),1,2); jg->addWidget(edJ4,1,3);
    jg->addWidget(new QLabel("J5:"),2,0); jg->addWidget(edJ5,2,1);
    jg->addWidget(new QLabel("J6:"),2,2); jg->addWidget(edJ6,2,3);
    tabs->addTab(jntTab, "Joints");
    vl->addWidget(tabs);

    // Buttons
    QHBoxLayout *btns = new QHBoxLayout(); btns->setSpacing(10);
    QPushButton *btnCancel = new QPushButton("✖ Cancel");
    btnCancel->setStyleSheet(S_BTN_RED);
    QPushButton *btnConfirm = new QPushButton("✅ Confirm");
    btnConfirm->setStyleSheet(S_BTN_GREEN);
    btns->addWidget(btnCancel); btns->addWidget(btnConfirm);
    vl->addLayout(btns);

    // Get actual_id from table S.No column
    int actualId = row + 1;
    if (m_tpTable->item(row, 0)) actualId = m_tpTable->item(row,0)->text().toInt();

    connect(btnCancel,  &QPushButton::clicked, dlg, &QDialog::reject);
    connect(btnConfirm, &QPushButton::clicked, dlg, [=]{
        if (!m_backend) { dlg->accept(); return; }
        QString mode = (tabs->currentIndex()==0) ? "CART" : "JOINT";
        m_backend->handleModifyTp(actualId, edName->text(),
            edX->text(), edY->text(), edZ->text(),
            edA->text(), edB->text(), edC->text(),
            edJ1->text(), edJ2->text(), edJ3->text(),
            edJ4->text(), edJ5->text(), edJ6->text(), mode);
        dlg->accept();
    });

    dlg->exec();
}

// ============================================================
//  showModifyPrDialog  –  replaces QML modifyInstPopup
// ============================================================
void RightPanel::showModifyPrDialog(int row)
{
    if (!m_prTable) return;

    QDialog *dlg = new QDialog(this);
    dlg->setWindowTitle("Modify Instruction");
    dlg->setStyleSheet(
        "QDialog{background:#1e1e2e;}"
        "QLabel{color:#B0BEC5;font-size:12px;background:transparent;}"
        "QComboBox{background:#0d1117;color:#e0e0e0;border:1px solid #00bcd4;"
        "border-radius:3px;padding:4px 8px;font-size:12px;}"
        "QComboBox QAbstractItemView{background:#0d1117;color:#e0e0e0;"
        "selection-background-color:#00bcd4;selection-color:#000;}"
        "QLineEdit{background:#0d1117;color:#e0e0e0;border:1px solid #00bcd4;"
        "border-radius:3px;padding:4px 8px;font-size:12px;}"
        "QLineEdit:focus{border-color:#80deea;}"
        "QPushButton{border-radius:4px;padding:8px 20px;font-weight:bold;font-size:12px;}"
    );
    dlg->setMinimumWidth(320);
    dlg->setAttribute(Qt::WA_DeleteOnClose);

    QVBoxLayout *vl = new QVBoxLayout(dlg); vl->setSpacing(12);

    QLabel *title = new QLabel("  MODIFY INSTRUCTION");
    title->setStyleSheet("color:#80deea;font-weight:bold;font-size:14px;"
                          "background:#0a1520;padding:8px;border-bottom:2px solid #00bcd4;");
    vl->addWidget(title);

    // Instruction type row
    QHBoxLayout *instRow = new QHBoxLayout();
    instRow->addWidget(new QLabel("Inst Type:"));
    QComboBox *cbInst = new QComboBox();
    cbInst->addItems({"MOVJ","MOVL","MOVC","ABSJ"});
    // Pre-select current instruction
    if (m_prTable->item(row,1)) {
        int idx = cbInst->findText(m_prTable->item(row,1)->text());
        if (idx >= 0) cbInst->setCurrentIndex(idx);
    }
    instRow->addWidget(cbInst,1);
    vl->addLayout(instRow);

    // Speed row
    QHBoxLayout *spdRow = new QHBoxLayout();
    spdRow->addWidget(new QLabel("Speed:"));
    QLineEdit *edSpd = new QLineEdit();
    edSpd->setValidator(new QDoubleValidator(0.0, 9999.0, 2, dlg));
    edSpd->setPlaceholderText("e.g. 150");
    if (m_prTable->item(row,4)) {
        QString v = m_prTable->item(row,4)->text();
        if(v != "--") edSpd->setText(v);
    }
    spdRow->addWidget(edSpd,1);
    vl->addLayout(spdRow);

    // Comment row
    QHBoxLayout *comRow = new QHBoxLayout();
    comRow->addWidget(new QLabel("Comment:"));
    QLineEdit *edCom = new QLineEdit();
    edCom->setPlaceholderText("Add a comment...");
    if (m_prTable->item(row,9)) {
        QString v = m_prTable->item(row,9)->text();
        if(v != "--" && v != "none") edCom->setText(v);
    }
    comRow->addWidget(edCom,1);
    vl->addLayout(comRow);

    vl->addStretch();

    // Buttons
    QHBoxLayout *btns = new QHBoxLayout(); btns->setSpacing(10);
    QPushButton *btnCancel  = new QPushButton("✖ Cancel");  btnCancel->setStyleSheet(S_BTN_RED);
    QPushButton *btnConfirm = new QPushButton("✅ Confirm"); btnConfirm->setStyleSheet(S_BTN_GREEN);
    btns->addWidget(btnCancel); btns->addWidget(btnConfirm);
    vl->addLayout(btns);

    int actualId = row + 1;
    if (m_prTable->item(row,0)) actualId = m_prTable->item(row,0)->text().toInt();

    connect(btnCancel,  &QPushButton::clicked, dlg, &QDialog::reject);
    connect(btnConfirm, &QPushButton::clicked, dlg, [=]{
        if (m_backend)
            m_backend->handleModifyPr(actualId, cbInst->currentText(), edSpd->text(), edCom->text());
        dlg->accept();
    });

    dlg->exec();
}

// ============================================================
//  buildSpeedPanel
// ============================================================
QWidget* RightPanel::buildSpeedPanel()
{
    QWidget *panel = new QWidget(this);
    panel->setStyleSheet("background:#151822;");
    QHBoxLayout *mainLayout = new QHBoxLayout(panel);
    mainLayout->setContentsMargins(20,10,20,10);

    const QString comboStyle = "QComboBox{background:white;color:black;font-weight:bold;padding:5px;border-radius:2px;min-width:100px;}";
    const QString spinStyle  = "QDoubleSpinBox{background:white;color:black;font-weight:bold;padding:5px;border-radius:2px;min-width:100px;}";
    const QString lblStyle   = "QLabel{color:white;font-weight:bold;font-size:13px;}";

    QWidget *formWidget = new QWidget(); QFormLayout *form = new QFormLayout(formWidget);
    form->setLabelAlignment(Qt::AlignRight|Qt::AlignVCenter); form->setSpacing(15);

    QComboBox *cmbMmInc = new QComboBox(); cmbMmInc->addItems({"mm","50","25","15","10","5","2","1","0.1","0.01","0.001"}); cmbMmInc->setStyleSheet(comboStyle);
    QDoubleSpinBox *spnMms = new QDoubleSpinBox(); spnMms->setRange(0.1,500.0); spnMms->setValue(50.0); spnMms->setStyleSheet(spinStyle);
    QComboBox *cmbDegInc = new QComboBox(); cmbDegInc->addItems({"deg","20","15","10","5","2","1","0.1","0.01","0.0001"}); cmbDegInc->setStyleSheet(comboStyle);
    QDoubleSpinBox *spnDegs = new QDoubleSpinBox(); spnDegs->setRange(0.1,500.0); spnDegs->setValue(50.0); spnDegs->setStyleSheet(spinStyle);
    QComboBox *cmbFrame = new QComboBox(); cmbFrame->addItems({"frames","Base","Tool","User"}); cmbFrame->setStyleSheet(comboStyle);

    connect(cmbMmInc,  &QComboBox::currentTextChanged,     this,[this](const QString &t){ if(m_backend) m_backend->setMmIncrement(t); });
    connect(spnMms,    &QDoubleSpinBox::valueChanged,       this,[this](double v){ if(m_backend) m_backend->setMmSpeed(v); });
    connect(cmbDegInc, &QComboBox::currentTextChanged,     this,[this](const QString &t){ if(m_backend) m_backend->setDegIncrement(t); });
    connect(spnDegs,   &QDoubleSpinBox::valueChanged,       this,[this](double v){ if(m_backend) m_backend->setDegSpeed(v); });
    connect(cmbFrame,  &QComboBox::currentTextChanged,     this,[this](const QString &t){ if(m_backend) m_backend->setFrame(t); });

    auto mkL = [&](const QString &t){ QLabel *l=new QLabel(t); l->setStyleSheet(lblStyle); return l; };
    form->addRow(mkL("MM Inc:"),  cmbMmInc); form->addRow(mkL("MM/S:"),    spnMms);
    form->addRow(mkL("DEG Inc:"), cmbDegInc); form->addRow(mkL("DEG/S:"), spnDegs);
    form->addRow(mkL("FRAME:"),   cmbFrame);

    QWidget *speedWidget = new QWidget(); QVBoxLayout *sl = new QVBoxLayout(speedWidget);
    sl->setAlignment(Qt::AlignCenter); sl->setSpacing(20);
    QLabel *lblTitle = new QLabel("GLOBAL SPEED");
    lblTitle->setStyleSheet("color:white;font-weight:bold;font-size:16px;letter-spacing:2px;");
    lblTitle->setAlignment(Qt::AlignCenter);
    QLabel *lblReadout = new QLabel("50%");
    lblReadout->setStyleSheet("color:#0EA5E9;font-weight:bold;font-size:32px;border:2px solid #0EA5E9;border-radius:8px;padding:10px 30px;background:#1E1E24;");
    lblReadout->setAlignment(Qt::AlignCenter);
    QSlider *slider = new QSlider(Qt::Horizontal); slider->setRange(1,100); slider->setValue(50);
    slider->setStyleSheet(
        "QSlider::groove:horizontal{border-radius:4px;height:8px;background:#2D2D30;}"
        "QSlider::sub-page:horizontal{background:qlineargradient(x1:0,y1:0,x2:1,y2:0,stop:0 #22C55E,stop:1 #0EA5E9);border-radius:4px;}"
        "QSlider::handle:horizontal{background:white;width:18px;height:18px;margin:-5px 0;border-radius:9px;}"
    );
    connect(slider,&QSlider::valueChanged,this,[this,lblReadout](int v){
        lblReadout->setText(QString::number(v)+"%");
        if(m_backend) m_backend->setGlobalSpeed(v);
    });
    sl->addWidget(lblTitle); sl->addWidget(lblReadout,0,Qt::AlignCenter); sl->addWidget(slider);

    mainLayout->addWidget(formWidget,1);
    QFrame *line=new QFrame(); line->setFrameShape(QFrame::VLine);
    line->setStyleSheet("background:#3E3E42;max-width:1px;border:none;");
    mainLayout->addWidget(line); mainLayout->addWidget(speedWidget,1);
    return panel;
}

// ============================================================
//  buildJointsPanel
// ============================================================
QWidget* RightPanel::buildJointsPanel()
{
    QWidget *panel = new QWidget(this);
    panel->setStyleSheet("background:#151822;");

    // ✅ Change to AlignTop so it doesn't float in the middle
    QVBoxLayout *mainLayout = new QVBoxLayout(panel);
    mainLayout->setAlignment(Qt::AlignTop | Qt::AlignHCenter);

    QGridLayout *grid = new QGridLayout();
    grid->setSpacing(8);

    QLabel *lN=new QLabel("NEG"); lN->setStyleSheet("color:#EF4444;font-weight:bold;"); lN->setAlignment(Qt::AlignCenter);
    QLabel *lB=new QLabel("BASE / WRIST"); lB->setStyleSheet("color:white;font-weight:bold;"); lB->setAlignment(Qt::AlignCenter);
    QLabel *lP=new QLabel("POS"); lP->setStyleSheet("color:#22C55E;font-weight:bold;"); lP->setAlignment(Qt::AlignCenter);
    grid->addWidget(lN,0,0); grid->addWidget(lB,0,1); grid->addWidget(lP,0,2);

    const QString bRed="QPushButton{background:#EF4444;color:white;font-weight:bold;font-size:18px;border-radius:4px;border-bottom:4px solid #B91C1C;min-width:95px;min-height:42px;}QPushButton:pressed{margin-top:4px;border-bottom:0;}";
    const QString bGrn="QPushButton{background:#4ADE80;color:white;font-weight:bold;font-size:18px;border-radius:4px;border-bottom:4px solid #16A34A;min-width:95px;min-height:42px;}QPushButton:pressed{margin-top:4px;border-bottom:0;}";
    const QString lblJ="QLabel{color:white;font-weight:bold;font-size:15px;}";

    for (int i=1;i<=6;++i) {
        QPushButton *bN=new QPushButton(QString("J%1-").arg(i)); bN->setStyleSheet(bRed);
        QLabel *lJ=new QLabel(QString("J%1").arg(i)); lJ->setStyleSheet(lblJ); lJ->setAlignment(Qt::AlignCenter);
        QPushButton *bP=new QPushButton(QString("J%1+").arg(i)); bP->setStyleSheet(bGrn);

        connect(bN,&QPushButton::pressed,this,[this,bN]{ if(m_backend) m_backend->handleButtonPress(bN->text()); });
        connect(bN,&QPushButton::released,this,[this,bN]{ if(m_backend) m_backend->handleButtonRelease(bN->text()); });
        connect(bP,&QPushButton::pressed,this,[this,bP]{ if(m_backend) m_backend->handleButtonPress(bP->text()); });
        connect(bP,&QPushButton::released,this,[this,bP]{ if(m_backend) m_backend->handleButtonRelease(bP->text()); });

        grid->addWidget(bN,i,0); grid->addWidget(lJ,i,1); grid->addWidget(bP,i,2);
    }
    mainLayout->addLayout(grid);
    mainLayout->addStretch(); // ✅ NEW: Pushes the joints grid tightly to the top!

    return panel;
}

// ============================================================
//  SLOTS
// ============================================================
void RightPanel::setGetPointsEnabled(bool enabled)
{
    if (!m_btnGetPoints) return;

    m_btnGetPoints->setEnabled(enabled);
    if (enabled) {
        // Active state: Bright Amber
        m_btnGetPoints->setStyleSheet(
            "QPushButton { background-color:#F59E0B; color:#000000; font-weight:bold; "
            "padding:12px; border-radius:4px; border:none; font-size:13px; }"
            "QPushButton:hover { background-color:#D97706; }"
            );
    } else {
        // Disabled state: Dark Grey
        m_btnGetPoints->setStyleSheet(
            "QPushButton { background-color:#2a3040; color:#64748b; font-weight:bold; "
            "padding:12px; border-radius:4px; border:none; font-size:13px; }"
            );
    }
}
void RightPanel::onTelemetryChanged()
{
    if (m_uiThrottle.elapsed() < 33) return;
    m_uiThrottle.restart();
    updateIOLeds();
}

void RightPanel::onProgramDataChanged()
{
    updatePrTable();
    updateTpTable();
    updateInstructionTable();
}

void RightPanel::onDirectoryDataChanged()
{
    updateOpPgDisplay();
    // Sync TP mode button label from backend
    if (m_backend && m_btnTpModeLabel) {
        QString mode = m_backend->property("tpRunModeName").toString();
        if (!mode.isEmpty()) m_btnTpModeLabel->setText("⚙ " + mode);
    }
}

void RightPanel::onLocalStateChanged()
{
    if (!m_backend || !m_varOutVal) return;
    m_varOutVal->setText(m_backend->property("variableOutputValue").toString());
}

void RightPanel::onHighlightChanged()
{
    if (!m_prTable || !m_backend) return;
    int hl = m_backend->property("highlightedInstruction").toInt();
    for (int i = 0; i < m_prTable->rowCount(); i++) {
        QColor c = (i == hl) ? QColor("#1565C0")
                             : QColor(i%2==0 ? "#1a1e2a" : "#151822");
        for (int col = 0; col < m_prTable->columnCount(); col++)
            if (auto *it = m_prTable->item(i,col)) it->setBackground(c);
    }
    if (hl >= 0 && hl < m_prTable->rowCount())
        m_prTable->scrollToItem(m_prTable->item(hl,0));
}

// ============================================================
//  UPDATE HELPERS
// ============================================================
void RightPanel::updateIOLeds()
{
    if (!m_backend) return;
    int di  = m_backend->property("digitalInputVal").toInt();
    int dov = m_backend->property("digitalOutputVal").toInt();
    for (int i = 0; i < 16; i++) {
        const bool diOn = (di >> i) & 1;
        if (m_diLeds[i])
            m_diLeds[i]->setStyleSheet(diOn
                ? "background:#4CAF50;border-radius:9px;border:1px solid #2E7D32;"
                : "background:#2a2d35;border-radius:9px;border:1px solid #3a3d45;");
        const bool doOn = (dov >> i) & 1;
        if (m_doLeds[i])
            m_doLeds[i]->setStyleSheet(doOn
                ? "background:#039BE5;border-radius:9px;border:1px solid #0277BD;"
                : "background:#2a2d35;border-radius:9px;border:1px solid #3a3d45;");
    }
}

void RightPanel::updateInstructionTable()
{
    if (!m_backend) return;
    auto get = [&](const char *prop) -> QString {
        QString v = m_backend->property(prop).toString();
        return v.isEmpty() ? "--" : v;
    };
    if (m_stgInst)    m_stgInst->setText(get("currentInstructionString"));
    if (m_stgName1)   m_stgName1->setText(get("stagingName1"));
    if (m_stgVal1)    m_stgVal1->setText(get("stagingValue1"));
    if (m_stgDeg1)    m_stgDeg1->setText(get("stagingDeg1"));
    if (m_stgName2)   m_stgName2->setText(get("stagingName2"));
    if (m_stgVal2)    m_stgVal2->setText(get("stagingValue2"));
    if (m_stgDeg2)    m_stgDeg2->setText(get("stagingDeg2"));
    if (m_stgSpeed)   m_stgSpeed->setText(get("stagingSpeed"));
    if (m_stgComment) m_stgComment->setText(get("stagingComment"));
}

void RightPanel::updatePrTable()
{
    if (!m_prTable || !m_backend) return;
    QVariantList data = m_backend->property("prProgramData").toList();
    int hl = m_backend->property("highlightedInstruction").toInt();

    m_prTable->setUpdatesEnabled(false);
    m_prTable->setRowCount(data.size());

    for (int i = 0; i < data.size(); i++) {
        QVariantMap it = data[i].toMap();
        auto mkItem = [](const QString &s) -> QTableWidgetItem* {
            auto *cell = new QTableWidgetItem(s.isEmpty() ? "--" : s);
            cell->setTextAlignment(Qt::AlignCenter);
            return cell;
        };
        m_prTable->setItem(i,0,  mkItem(QString::number(i+1)));
        m_prTable->setItem(i,1,  mkItem(it.value("inst","MOVL").toString()));
        m_prTable->setItem(i,2,  mkItem(it.value("name").toString()));
        m_prTable->setItem(i,3,  mkItem(it.value("value").toString()));
        m_prTable->setItem(i,4,  mkItem(it.value("speed","--").toString()));
        m_prTable->setItem(i,5,  mkItem(it.value("deg","--").toString()));
        m_prTable->setItem(i,6,  mkItem(it.value("rad","--").toString()));
        m_prTable->setItem(i,7,  mkItem(it.value("tool","--").toString()));
        m_prTable->setItem(i,8,  mkItem(it.value("frame","--").toString()));
        m_prTable->setItem(i,9,  mkItem(it.value("comt","--").toString()));
        m_prTable->setItem(i,10, mkItem(it.value("dist","--").toString()));
        m_prTable->setItem(i,11, mkItem(it.value("time","--").toString()));

        // Row coloring: highlighted = blue, alternating dark rows
        QColor bg = (i == hl)
            ? QColor("#1565C0")
            : QColor(i%2==0 ? "#0d1117" : "#111520");
        for (int c = 0; c < 12; c++) {
            if (auto *cell = m_prTable->item(i,c)) {
                cell->setBackground(bg);
                // Dim non-essential columns slightly
                if (i != hl && (c==6||c==7||c==8||c==10||c==11))
                    cell->setForeground(QColor("#607080"));
            }
        }
    }
    m_prTable->setUpdatesEnabled(true);
}

void RightPanel::updateTpTable()
{
    if (!m_tpTable || !m_backend) return;
    QVariantList data = m_backend->property("tpPointData").toList();

    m_tpTable->setUpdatesEnabled(false);
    m_tpTable->setRowCount(data.size());

    for (int i = 0; i < data.size(); i++) {
        QVariantMap it = data[i].toMap();
        auto mkItem = [](const QString &s) -> QTableWidgetItem* {
            auto *cell = new QTableWidgetItem(s.isEmpty() ? "--" : s);
            cell->setTextAlignment(Qt::AlignCenter);
            return cell;
        };
        m_tpTable->setItem(i,0, mkItem(QString::number(i+1)));
        m_tpTable->setItem(i,1, mkItem(it.value("name", QString("tp%1").arg(i+1)).toString()));
        m_tpTable->setItem(i,2, mkItem(it.value("value").toString()));
        m_tpTable->setItem(i,3, mkItem(it.value("deg","--").toString()));
        m_tpTable->setItem(i,4, mkItem(it.value("tool","--").toString()));
        m_tpTable->setItem(i,5, mkItem(it.value("frame","--").toString()));

        QColor bg(i%2==0 ? "#0d1117" : "#111520");
        for (int c = 0; c < 6; c++)
            if (auto *cell = m_tpTable->item(i,c)) cell->setBackground(bg);
    }
    m_tpTable->setUpdatesEnabled(true);
}

void RightPanel::updateOpPgDisplay()
{
    if (!m_backend || !m_lblOpPg) return;
    m_lblOpPg->setText(m_backend->property("programCountOutput").toString());
}

// ============================================================
//  setActiveTab
// ============================================================
void RightPanel::setActiveTab(int index)
{
    if (m_workspaceTabs) m_workspaceTabs->setCurrentIndex(index);
}

// ============================================================
//  resizeEvent
// ============================================================
void RightPanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    if (tray && !tray->isHidden()) {
        const int tw = 280;
        tray->setGeometry(this->width()-tw, 0, tw, this->height());
    }
}

// ============================================================
//  eventFilter  –  swipe detection on m_controlSwipeStack
// ============================================================
bool RightPanel::eventFilter(QObject *watched, QEvent *event)
{
    Q_UNUSED(watched);

    if (!m_controlSwipeStack || m_controlSwipeStack->isHidden())
        return QWidget::eventFilter(watched, event);

    if (event->type() == QEvent::MouseButtonPress) {
        auto *me = static_cast<QMouseEvent*>(event);
        QPoint gp = me->globalPosition().toPoint();
        QPoint tl = m_controlSwipeStack->mapToGlobal(QPoint(0,0));
        QRect  rect(tl, m_controlSwipeStack->size());
        if (rect.contains(gp)) {
            m_ctrlSwipeStart    = gp;
            m_ctrlSwipeTracking = true;
        }
    } else if (event->type() == QEvent::MouseButtonRelease && m_ctrlSwipeTracking) {
        auto *me = static_cast<QMouseEvent*>(event);
        int dx = me->globalPosition().toPoint().x() - m_ctrlSwipeStart.x();
        if (qAbs(dx) > 80) {
            m_controlSwipeStack->setCurrentIndex(dx < 0 ? 1 : 0);
            emit requestFooterSwipe();
        }
        m_ctrlSwipeTracking = false;
    }

    return QWidget::eventFilter(watched, event);
}