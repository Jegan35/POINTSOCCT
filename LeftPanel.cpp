#include "LeftPanel.h"
#include <QGridLayout>
#include <QTimer> // Needed for the auto-load delay
#include <QMenu>
#include <QAction>
#include <QApplication>
#include <QMouseEvent>
#include <QEvent>
#include <QPushButton>
#include <QDialog>
#include <QLineEdit>     // ✅ For the Filename input
#include <QListWidget>   // ✅ For the Open/Delete file lists

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
    // BOTTOM SECTION 2: Swappable Footer Stack
    // ==========================================
    footerStack = new QStackedWidget(this);

    // ------------------------------------------
    // PAGE 0: Original Controls
    // ------------------------------------------
    QWidget *page0 = new QWidget();
    QGridLayout *footerGrid = new QGridLayout(page0); // ✅ Declared ONCE and attached to page0
    footerGrid->setSpacing(5);
    footerGrid->setContentsMargins(0,0,0,0);

    QString baseBtn = "color: white; font-weight: bold; font-size: 14px; min-height: 45px; border-radius: 3px;";

    // --- ROW 1 (6 Buttons) ---
    QPushButton *btnServo = new QPushButton("OFF", this);
    btnServo->setStyleSheet("background-color: #4B5563; " + baseBtn);
    connect(btnServo, &QPushButton::clicked, [btnServo, baseBtn]() {
        if(btnServo->text() == "OFF") { btnServo->setText("ON"); btnServo->setStyleSheet("background-color: #10B981; " + baseBtn); }
        else { btnServo->setText("OFF"); btnServo->setStyleSheet("background-color: #4B5563; " + baseBtn); }
    });

    QPushButton *btnHome = new QPushButton("HOME", this);
    btnHome->setStyleSheet("background-color: #007ACC; " + baseBtn);

    QPushButton *btnRun = new QPushButton("RUN", this);
    btnRun->setStyleSheet("background-color: #10B981; " + baseBtn);
    connect(btnRun, &QPushButton::clicked, [btnRun, baseBtn]() {
        if(btnRun->text() == "RUN") { btnRun->setText("PAUSE"); btnRun->setStyleSheet("background-color: #F59E0B; " + baseBtn); }
        else { btnRun->setText("RUN"); btnRun->setStyleSheet("background-color: #10B981; " + baseBtn); }
    });

    QPushButton *btnStart = new QPushButton("START", this);
    btnStart->setStyleSheet("background-color: #4B5563; " + baseBtn);
    connect(btnStart, &QPushButton::clicked, [btnStart, baseBtn]() {
        if(btnStart->text() == "START") { btnStart->setText("STOP"); btnStart->setStyleSheet("background-color: #EF4444; " + baseBtn); }
        else { btnStart->setText("START"); btnStart->setStyleSheet("background-color: #4B5563; " + baseBtn); }
    });

    QPushButton *btnExit = new QPushButton("EXIT", this);
    btnExit->setStyleSheet("background-color: #991B1B; " + baseBtn);

    // --- POPUP MENU BUTTON ---
    QPushButton *btnMode = new QPushButton("M: SIM", this);
    QString btnModeStyle = baseBtn + " QPushButton::menu-indicator { image: none; }";
    btnMode->setStyleSheet("background-color: #059669; " + btnModeStyle);

    QMenu *modeMenu = new QMenu(btnMode);
    modeMenu->setStyleSheet("QMenu { background-color: #2D2D30; color: white; border: 1px solid #3E3E42; font-weight: bold; font-size: 14px; } "
                            "QMenu::item { padding: 10px 30px; } "
                            "QMenu::item:selected { background-color: #007ACC; }");

    QAction *actSim = new QAction("M: SIM", this);
    QAction *actReal = new QAction("M: REAL", this);
    modeMenu->addAction(actSim);
    modeMenu->addAction(actReal);
    btnMode->setMenu(modeMenu);

    connect(actSim, &QAction::triggered, [btnMode, btnModeStyle]() {
        btnMode->setText("M: SIM");
        btnMode->setStyleSheet("background-color: #059669; " + btnModeStyle);
    });

    connect(actReal, &QAction::triggered, [btnMode, btnModeStyle]() {
        btnMode->setText("M: REAL");
        btnMode->setStyleSheet("background-color: #D97706; " + btnModeStyle);
    });

    // Add Row 1 to Grid
    footerGrid->addWidget(btnServo, 0, 0);
    footerGrid->addWidget(btnHome,  0, 1);
    footerGrid->addWidget(btnRun,   0, 2);
    footerGrid->addWidget(btnStart, 0, 3);
    footerGrid->addWidget(btnExit,  0, 4);
    footerGrid->addWidget(btnMode,  0, 5);

    // --- ROW 2 (5 Elements) ---
    QPushButton *btnSysOk = new QPushButton("SYSTEM OK", this);
    btnSysOk->setStyleSheet("background-color: #10B981; " + baseBtn);

    QPushButton *btnErrClr = new QPushButton("ERRCLR", this);
    btnErrClr->setStyleSheet("background-color: #EF4444; " + baseBtn);

    QPushButton *btnMrkClr = new QPushButton("MRKCLR", this);
    btnMrkClr->setStyleSheet("background-color: #F59E0B; " + baseBtn);

    QPushButton *btnReset = new QPushButton("RESET", this);
    btnReset->setStyleSheet("background-color: #DC2626; " + baseBtn);

    QPushButton *btnSpeed = new QPushButton("SPEED", this);
    btnSpeed->setStyleSheet("background-color: #064E3B; " + baseBtn);

    // Add Row 2 to Grid
    footerGrid->addWidget(btnSysOk,  1, 0, 1, 2);
    footerGrid->addWidget(btnErrClr, 1, 2);
    footerGrid->addWidget(btnMrkClr, 1, 3);
    footerGrid->addWidget(btnReset,  1, 4);
    footerGrid->addWidget(btnSpeed,  1, 5);

    // ------------------------------------------
    // PAGE 1: The New 'Action/Tools' Grid
    // ------------------------------------------
    QWidget *page1 = new QWidget();
    QGridLayout *swipeGrid = new QGridLayout(page1);
    swipeGrid->setSpacing(5);
    swipeGrid->setContentsMargins(0,0,0,0);

    QString darkBox = "QLabel { background-color: #111827; color: #9CA3AF; font-weight: bold; padding: 12px; border-radius: 4px; border: 1px solid #3E3E42; }";
    QString prBox = "QLabel { background-color: #111827; color: #9CA3AF; font-weight: bold; padding: 12px; border-radius: 4px; border: 2px solid #00E5FF; }";
    QString actBtn = "QPushButton { background-color: #475569; color: white; font-weight: bold; padding: 15px; border-radius: 4px; border-bottom: 4px solid #334155; } QPushButton:pressed { margin-top: 4px; border-bottom: 0px; }";

    // ==========================================
    // 📂 FILES BUTTON & INLINE POPUP (WITH SEARCH)
    // ==========================================
    QPushButton *btnFiles = new QPushButton("📂 FILES", page1);
    btnFiles->setStyleSheet("background-color: #8B5CF6; color: white; font-weight: bold; padding: 12px; border-radius: 4px; border: 1px solid #3E3E42;");

    connect(btnFiles, &QPushButton::clicked, this, [this, btnFiles]() {
        QDialog dialog(this);
        // Made slightly taller to fit the search bar
        dialog.setFixedSize(350, 460);
        dialog.setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
        dialog.setStyleSheet("QDialog { background-color: #1e1e2e; border: 2px solid #3f3f5f; border-radius: 8px; }");

        QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);
        mainLayout->setContentsMargins(15, 15, 15, 15);
        QStackedWidget *stack = new QStackedWidget(&dialog);

        QString activeCategory = "TP";

        QString baseStyle =
            "QPushButton { color: white; font-weight: bold; font-size: 15px; border-radius: 6px; padding: 12px; min-height: 25px; letter-spacing: 1px; } "
            "QPushButton:pressed { margin-top: 5px; border-bottom: 0px; } "
            "QPushButton:disabled { background-color: #2D2D30; color: #555555; border-bottom: 5px solid #1E1E24; }";

        // ==========================================
        // --- PAGE 0: TYPE SELECT ---
        // ==========================================
        QWidget *page0 = new QWidget();
        QVBoxLayout *p0Layout = new QVBoxLayout(page0);
        p0Layout->setSpacing(12);

        QLabel *lblTitle0 = new QLabel("SELECT TYPE");
        lblTitle0->setStyleSheet("color: #B0BEC5; font-weight: bold; font-size: 14px; border: none;");
        lblTitle0->setAlignment(Qt::AlignCenter);

        QPushButton *btnTp = new QPushButton("TARGET POINT FILES"); btnTp->setStyleSheet(baseStyle + "QPushButton { background-color: #00897B; border-bottom: 5px solid #004D40; }");
        QPushButton *btnPr = new QPushButton("PROGRAM FILES"); btnPr->setStyleSheet(baseStyle + "QPushButton { background-color: #039BE5; border-bottom: 5px solid #01579B; }");
        QPushButton *btnTr = new QPushButton("TRAJECTORY FILES"); btnTr->setStyleSheet(baseStyle + "QPushButton { background-color: #7E57C2; border-bottom: 5px solid #4527A0; }");
        QPushButton *btnClose = new QPushButton("CLOSE"); btnClose->setStyleSheet(baseStyle + "QPushButton { background-color: #E53935; border-bottom: 5px solid #b71c1c; }");

        p0Layout->addWidget(lblTitle0); p0Layout->addWidget(btnTp); p0Layout->addWidget(btnPr); p0Layout->addWidget(btnTr);
        p0Layout->addStretch(); p0Layout->addWidget(btnClose);

        // ==========================================
        // --- PAGE 1: OPERATIONS ---
        // ==========================================
        QWidget *page1 = new QWidget();
        QVBoxLayout *p1Layout = new QVBoxLayout(page1);
        p1Layout->setSpacing(12);

        QLabel *lblTitle1 = new QLabel("OPERATIONS");
        lblTitle1->setStyleSheet("color: #B0BEC5; font-weight: bold; font-size: 14px; border: none;");
        lblTitle1->setAlignment(Qt::AlignCenter);

        QPushButton *btnNew = new QPushButton("NEW FILE"); btnNew->setStyleSheet(baseStyle + "QPushButton { background-color: #43A047; border-bottom: 5px solid #1B5E20; }");
        QPushButton *btnOpen = new QPushButton("OPEN FILE"); btnOpen->setStyleSheet(baseStyle + "QPushButton { background-color: #039BE5; border-bottom: 5px solid #01579B; }");
        QPushButton *btnDelete = new QPushButton("DELETE FILE"); btnDelete->setStyleSheet(baseStyle + "QPushButton { background-color: #E53935; border-bottom: 5px solid #b71c1c; }");
        QPushButton *btnBack1 = new QPushButton("BACK"); btnBack1->setStyleSheet(baseStyle + "QPushButton { background-color: #555566; border-bottom: 5px solid #333344; }");

        p1Layout->addWidget(lblTitle1); p1Layout->addWidget(btnNew); p1Layout->addWidget(btnOpen); p1Layout->addWidget(btnDelete);
        p1Layout->addStretch(); p1Layout->addWidget(btnBack1);

        // ==========================================
        // --- PAGE 2: CREATE NEW FILE ---
        // ==========================================
        QWidget *page2 = new QWidget();
        QVBoxLayout *p2Layout = new QVBoxLayout(page2);
        p2Layout->setSpacing(12);

        QLabel *lblTitle2 = new QLabel("CREATE NEW FILE");
        lblTitle2->setStyleSheet("color: #00E676; font-weight: bold; font-size: 14px; border: none;");
        lblTitle2->setAlignment(Qt::AlignCenter);

        QLabel *lblInputBox = new QLabel("FILENAME");
        lblInputBox->setStyleSheet("color: #8888aa; font-weight: bold; font-size: 10px; border: none;");

        QLineEdit *txtFilename = new QLineEdit();
        txtFilename->setPlaceholderText("e.g., Weld_Path_01");
        txtFilename->setStyleSheet("QLineEdit { background-color: #0a0a12; color: white; border: 1px solid #3f3f5f; border-radius: 4px; padding: 12px; font-family: monospace; font-size: 16px; } QLineEdit:focus { border: 2px solid #00E676; }");

        QHBoxLayout *p2BtnLayout = new QHBoxLayout();
        p2BtnLayout->setSpacing(10);
        QPushButton *btnCancelCreate = new QPushButton("CANCEL"); btnCancelCreate->setStyleSheet(baseStyle + "QPushButton { background-color: #D32F2F; border-bottom: 5px solid #b71c1c; padding: 10px; }");
        QPushButton *btnSubmitCreate = new QPushButton("CREATE"); btnSubmitCreate->setStyleSheet(baseStyle + "QPushButton { background-color: #2E7D32; border-bottom: 5px solid #1B5E20; padding: 10px; }");
        btnSubmitCreate->setEnabled(false);

        connect(txtFilename, &QLineEdit::textChanged, [btnSubmitCreate](const QString &text) {
            btnSubmitCreate->setEnabled(!text.trimmed().isEmpty());
        });

        p2BtnLayout->addWidget(btnCancelCreate); p2BtnLayout->addWidget(btnSubmitCreate);
        p2Layout->addWidget(lblTitle2); p2Layout->addWidget(lblInputBox); p2Layout->addWidget(txtFilename);
        p2Layout->addStretch(); p2Layout->addLayout(p2BtnLayout);

        // ==========================================
        // --- PAGE 3: OPEN / DELETE FILE LIST (NOW WITH SEARCH) ---
        // ==========================================
        QWidget *page3 = new QWidget();
        QVBoxLayout *p3Layout = new QVBoxLayout(page3);
        p3Layout->setSpacing(10);

        QLabel *lblTitle3 = new QLabel("SELECT FILE");
        lblTitle3->setStyleSheet("color: #039BE5; font-weight: bold; font-size: 14px; border: none;");
        lblTitle3->setAlignment(Qt::AlignCenter);

        // ✅ NEW: The Search Bar
        QLineEdit *txtSearch = new QLineEdit();
        txtSearch->setPlaceholderText("🔍 Search files...");
        txtSearch->setStyleSheet("QLineEdit { background-color: #0a0a12; color: white; border: 1px solid #3f3f5f; border-radius: 4px; padding: 8px 12px; font-family: monospace; font-size: 14px; } QLineEdit:focus { border: 2px solid #039BE5; }");

        QListWidget *fileList = new QListWidget();
        fileList->setStyleSheet("QListWidget { background-color: #0a0a12; color: white; border: 1px solid #3f3f5f; border-radius: 4px; padding: 5px; font-size: 14px; } QListWidget::item { padding: 12px; border-bottom: 1px solid #222233; } QListWidget::item:selected { background-color: #2979FF; border-radius: 4px; }");

        // Dummy Data (Map to robotController later)
        fileList->addItem("Program_A_Weld");
        fileList->addItem("Program_B_Pallet");
        fileList->addItem("Test_Sequence_01");
        fileList->addItem("Pallet_Layer_2");
        fileList->addItem("Maintenance_Home");

        // ✅ NEW: Search Filter Logic
        connect(txtSearch, &QLineEdit::textChanged, [fileList](const QString &text) {
            for (int i = 0; i < fileList->count(); ++i) {
                QListWidgetItem *item = fileList->item(i);
                // Hide the item if it doesn't match the search text (Case Insensitive)
                item->setHidden(!item->text().contains(text, Qt::CaseInsensitive));
            }
        });

        QHBoxLayout *p3BtnLayout = new QHBoxLayout();
        p3BtnLayout->setSpacing(10);
        QPushButton *btnCancelList = new QPushButton("CANCEL"); btnCancelList->setStyleSheet(baseStyle + "QPushButton { background-color: #D32F2F; border-bottom: 5px solid #b71c1c; padding: 10px; }");
        QPushButton *btnSubmitList = new QPushButton("ACTION"); btnSubmitList->setStyleSheet(baseStyle + "QPushButton { background-color: #039BE5; border-bottom: 5px solid #01579B; padding: 10px; }");
        btnSubmitList->setEnabled(false);

        connect(fileList, &QListWidget::itemSelectionChanged, [btnSubmitList, fileList]() {
            btnSubmitList->setEnabled(fileList->selectedItems().count() > 0);
        });

        p3BtnLayout->addWidget(btnCancelList); p3BtnLayout->addWidget(btnSubmitList);

        // Add elements to the layout, including the new search bar
        p3Layout->addWidget(lblTitle3);
        p3Layout->addWidget(txtSearch); // <--- Added here
        p3Layout->addWidget(fileList);
        p3Layout->addLayout(p3BtnLayout);

        // --- ADD TO STACK ---
        stack->addWidget(page0); stack->addWidget(page1); stack->addWidget(page2); stack->addWidget(page3);
        mainLayout->addWidget(stack);

        // ==========================================
        // --- DYNAMIC WIRING ---
        // ==========================================
        connect(btnTp, &QPushButton::clicked, [&]() { activeCategory = "TP"; lblTitle1->setText("TP OPERATIONS"); stack->setCurrentIndex(1); });
        connect(btnPr, &QPushButton::clicked, [&]() { activeCategory = "PR"; lblTitle1->setText("PR OPERATIONS"); stack->setCurrentIndex(1); });
        connect(btnTr, &QPushButton::clicked, [&]() { activeCategory = "TR"; lblTitle1->setText("TR OPERATIONS"); stack->setCurrentIndex(1); });

        connect(btnNew, &QPushButton::clicked, [&]() {
            lblTitle2->setText("CREATE NEW " + activeCategory);
            txtFilename->clear();
            stack->setCurrentIndex(2);
        });

        connect(btnOpen, &QPushButton::clicked, [&]() {
            lblTitle3->setText("OPEN " + activeCategory + " FILE");
            lblTitle3->setStyleSheet("color: #039BE5; font-weight: bold; font-size: 14px; border: none;");
            btnSubmitList->setText("OPEN");
            btnSubmitList->setStyleSheet(baseStyle + "QPushButton { background-color: #039BE5; border-bottom: 5px solid #01579B; padding: 10px; }");
            txtSearch->setStyleSheet("QLineEdit { background-color: #0a0a12; color: white; border: 1px solid #3f3f5f; border-radius: 4px; padding: 8px 12px; font-family: monospace; font-size: 14px; } QLineEdit:focus { border: 2px solid #039BE5; }");

            txtSearch->clear(); // ✅ Clear search bar
            fileList->clearSelection();
            stack->setCurrentIndex(3);
        });

        connect(btnDelete, &QPushButton::clicked, [&]() {
            lblTitle3->setText("DELETE " + activeCategory + " FILE");
            lblTitle3->setStyleSheet("color: #E53935; font-weight: bold; font-size: 14px; border: none;");
            btnSubmitList->setText("DELETE");
            btnSubmitList->setStyleSheet(baseStyle + "QPushButton { background-color: #E53935; border-bottom: 5px solid #b71c1c; padding: 10px; }");
            txtSearch->setStyleSheet("QLineEdit { background-color: #0a0a12; color: white; border: 1px solid #3f3f5f; border-radius: 4px; padding: 8px 12px; font-family: monospace; font-size: 14px; } QLineEdit:focus { border: 2px solid #E53935; }"); // Changes border focus color to Red

            txtSearch->clear(); // ✅ Clear search bar
            fileList->clearSelection();
            stack->setCurrentIndex(3);
        });

        connect(btnClose, &QPushButton::clicked, &dialog, &QDialog::reject);
        connect(btnBack1, &QPushButton::clicked, [&]() { stack->setCurrentIndex(0); });
        connect(btnCancelCreate, &QPushButton::clicked, [&]() { stack->setCurrentIndex(1); });
        connect(btnCancelList, &QPushButton::clicked, [&]() { stack->setCurrentIndex(1); });

        connect(btnSubmitCreate, &QPushButton::clicked, &dialog, &QDialog::accept);
        connect(btnSubmitList, &QPushButton::clicked, &dialog, &QDialog::accept);

        // --- POPUP POSITIONING ---
        QPoint btnGlobalPos = btnFiles->mapToGlobal(QPoint(0, 0));
        dialog.setGeometry(btnGlobalPos.x(), btnGlobalPos.y() - 470, 350, 460); // Adjusted for taller height

        dialog.exec();
    });

    QLabel *lblTP = new QLabel("TP: None", page1); lblTP->setStyleSheet(darkBox);
    QLabel *lblPR = new QLabel("PR: None", page1); lblPR->setStyleSheet(prBox);
    QLabel *lblTR = new QLabel("TR: None", page1); lblTR->setStyleSheet(darkBox);

    QPushButton *btnTools = new QPushButton("+ TOOLS", page1);
    btnTools->setStyleSheet("background-color: #EC4899; color: white; font-weight: bold; padding: 12px; border-radius: 4px;");

    QLabel *lblToolName = new QLabel("Tool Name...", page1); lblToolName->setStyleSheet(darkBox);

    // Row 2
    QPushButton *btnAct1 = new QPushButton("ACTION 1", page1); btnAct1->setStyleSheet(actBtn);
    QPushButton *btnAct2 = new QPushButton("ACTION 2", page1); btnAct2->setStyleSheet(actBtn);
    QPushButton *btnAct3 = new QPushButton("ACTION 3", page1); btnAct3->setStyleSheet(actBtn);
    QPushButton *btnAct4 = new QPushButton("ACTION 4", page1); btnAct4->setStyleSheet(actBtn);
    QPushButton *btnAct5 = new QPushButton("ACTION 5", page1); btnAct5->setStyleSheet(actBtn);
    QPushButton *btnAct6 = new QPushButton("ACTION 6", page1); btnAct6->setStyleSheet(actBtn);

    swipeGrid->addWidget(btnFiles, 0, 0);
    swipeGrid->addWidget(lblTP, 0, 1);
    swipeGrid->addWidget(lblPR, 0, 2);
    swipeGrid->addWidget(lblTR, 0, 3);
    swipeGrid->addWidget(btnTools, 0, 4);
    swipeGrid->addWidget(lblToolName, 0, 5);

    swipeGrid->addWidget(btnAct1, 1, 0);
    swipeGrid->addWidget(btnAct2, 1, 1);
    swipeGrid->addWidget(btnAct3, 1, 2);
    swipeGrid->addWidget(btnAct4, 1, 3);
    swipeGrid->addWidget(btnAct5, 1, 4);
    swipeGrid->addWidget(btnAct6, 1, 5);

    // ✅ ADD PAGES TO STACK
    footerStack->addWidget(page0);
    footerStack->addWidget(page1);

    // ✅ ADD STACK TO MAIN LAYOUT (Not the grid directly)
    mainLayout->addWidget(footerStack);

    // ==========================================
    // SAFE AUTO-LOAD TRIGGER
    // ==========================================
    QTimer::singleShot(500, myMainWidget, &OcctWidget::loadDefaultRobot);
    qApp->installEventFilter(this); // Tell the app to send all mouse events here first
    dragStartPos = QPoint(-1, -1);
}
// Ensure this code is in LeftPanel.cpp
void LeftPanel::toggleFooterSwipe()
{
    if(footerStack->currentIndex() == 0) {
        footerStack->setCurrentIndex(1);
    } else {
        footerStack->setCurrentIndex(0);
    }
}
void LeftPanel::setSwipeEnabled(bool enabled)
{
    isSwipeUnlocked = enabled;
}
bool LeftPanel::eventFilter(QObject *watched, QEvent *event)
{
    // If the switch is OFF, do nothing. Let the UI act normally.
    if (!isSwipeUnlocked) return false;

    if (event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);

        // Check if the click happened INSIDE the footer stack area
        QPoint globalPos = mouseEvent->globalPosition().toPoint();
        QPoint footerTopLeft = footerStack->mapToGlobal(QPoint(0, 0));
        QRect globalFooterRect(footerTopLeft, footerStack->size());

        if (globalFooterRect.contains(globalPos)) {
            dragStartPos = globalPos; // Record where the swipe started
        } else {
            dragStartPos = QPoint(-1, -1);
        }
    }
    else if (event->type() == QEvent::MouseButtonRelease) {
        if (dragStartPos.x() != -1) {
            QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
            QPoint globalPos = mouseEvent->globalPosition().toPoint();
            int dragDistance = globalPos.x() - dragStartPos.x();

            // If the user dragged left or right by more than 80 pixels
            if (qAbs(dragDistance) > 80) {
                if (dragDistance > 0) {
                    footerStack->setCurrentIndex(0); // Swiped Right
                } else {
                    footerStack->setCurrentIndex(1); // Swiped Left
                }
                dragStartPos = QPoint(-1, -1);
                return true; // ✅ Eat the event! Prevents buttons from being clicked!
            }
        }
        dragStartPos = QPoint(-1, -1);
    }

    // Let all other events pass through normally
    return QWidget::eventFilter(watched, event);
}

class IndustrialButton : public QPushButton {
public:
    IndustrialButton(const QString &text, QWidget *parent = nullptr) : QPushButton(text, parent) {
        setCheckable(true);
        setStyleSheet(
            "QPushButton { "
            "  background-color: #2D2D30; color: #9CA3AF; "
            "  font-weight: bold; padding: 10px; border-radius: 4px; "
            "  border: 1px solid #3E3E42; "
            "} "
            "QPushButton:checked { "
            "  background-color: #00E5FF; color: black; border: 1px solid #00E5FF; "
            "}"
            );
    }
};