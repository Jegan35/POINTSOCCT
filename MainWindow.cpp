// ==========================================
// 1. ALL QT HEADERS MUST BE INCLUDED FIRST
// ==========================================
#include <QToolBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFileDialog>
#include <QStatusBar>
#include <QComboBox>
#include <QSplitter>
#include <QTextEdit>
#include "kinematic.h"
// Add these with your other Qt headers at the top of MainWindow.cpp!
#include <QTabWidget>
#include <QRadioButton>
#include <QGridLayout>
#include <QGroupBox>
#include <QLabel>
// ✅ NEW: Headers required for the Position Part dialog
#include <QDialog>
#include <QFormLayout>
#include <QDoubleSpinBox>
#include <QDialogButtonBox>
#include <QTimer>
// ==========================================
// 2. LOCAL HEADER INCLUDED SECOND
// ==========================================
#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // ==========================================
    // 1. BUILD THE 70/30 & 50/50 SPLIT UI
    // ==========================================
    QSplitter *mainSplitter = new QSplitter(Qt::Horizontal, this);
    setCentralWidget(mainSplitter);

    // LEFT PANEL (70%) - Main 3D View
    myMainWidget = new OcctWidget(this);
    myMainWidget->setViewRole(OcctWidget::MainRole); // ✅ Tell it to act like the Main View
    mainSplitter->addWidget(myMainWidget);

    // RIGHT PANEL (30%) - Contains 50/50 vertical split
    QWidget *rightPanel = new QWidget(this);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    QSplitter *rightSplitter = new QSplitter(Qt::Vertical, rightPanel);

    // Top 50% of Right Panel: Isolated 3D View
    mySideWidget = new OcctWidget(this);
    mySideWidget->setViewRole(OcctWidget::SideRole); // ✅ Tell it to act like the Side View
    rightSplitter->addWidget(mySideWidget);

    // ==========================================
    // ✅ BOTTOM 50% TABS (XYZ Log & Jog Controls)
    // ==========================================
    QTabWidget *bottomTabs = new QTabWidget(rightPanel);
    bottomTabs->setStyleSheet(
        "QTabWidget::pane { border: 2px solid #3E3E42; background: #2D2D30; } "
        "QTabBar::tab { background: #1E1E1E; color: white; padding: 8px 20px; font-weight: bold; border-right: 1px solid #3E3E42; } "
        "QTabBar::tab:selected { background: #007ACC; color: white; }"
        );

    // --- TAB 1: The XYZ Coordinate Log ---
    myCoordinateLog = new QTextEdit(bottomTabs);
    myCoordinateLog->setReadOnly(true);
    myCoordinateLog->setStyleSheet("background-color: #1E1E1E; color: #00FF00; font-family: monospace; border: none;");
    bottomTabs->addTab(myCoordinateLog, "XYZ Paths");

    // --- TAB 2: The Robot Jog / Move Controller ---
    QWidget *jogTab = new QWidget(bottomTabs);
    QVBoxLayout *jogLayout = new QVBoxLayout(jogTab);

    // Top Section: Jog vs Move
    QGroupBox *grpMode = new QGroupBox("Operation Mode");
    grpMode->setStyleSheet("color: white; font-weight: bold;");
    QHBoxLayout *modeLayout = new QHBoxLayout(grpMode);
    QRadioButton *rdoJog = new QRadioButton("JOG (Continuous)");
    QRadioButton *rdoMove = new QRadioButton("MOVE (To Destination)");
    rdoJog->setChecked(true); // Default to JOG
    modeLayout->addWidget(rdoJog);
    modeLayout->addWidget(rdoMove);
    jogLayout->addWidget(grpMode);

    // ==========================================
    // ✅ MIDDLE SECTION: JOG STEP & SPEED TRAY
    // ==========================================
    QGroupBox *grpUnits = new QGroupBox("Step Size & Speed");
    grpUnits->setStyleSheet("color: white; font-weight: bold;");
    QGridLayout *unitLayout = new QGridLayout(grpUnits);

    QString comboStyle = "QComboBox { background-color: #1E1E1E; color: white; padding: 3px; border: 1px solid #3E3E42; }";

    // 1. Degree Combobox
    QLabel *lblDeg = new QLabel("Deg:");
    QComboBox *cmbDeg = new QComboBox();
    cmbDeg->addItems({"20", "15", "10", "5", "2", "1", "0.1", "0.01", "0.001", "0.0001"});
    cmbDeg->setEditable(true); // Allows typing custom numbers!
    cmbDeg->setCurrentText("1");
    cmbDeg->setStyleSheet(comboStyle);

    // 2. mm Combobox
    QLabel *lblMm = new QLabel("mm:");
    QComboBox *cmbMm = new QComboBox();
    cmbMm->addItems({"50", "25", "15", "10", "5", "2", "1", "0.1", "0.01", "0.001"});
    cmbMm->setEditable(true);
    cmbMm->setCurrentText("10");
    cmbMm->setStyleSheet(comboStyle);

    // 3. DPS (Degrees Per Second) Combobox
    QLabel *lblDps = new QLabel("DPS:");
    QComboBox *cmbDps = new QComboBox();
    cmbDps->addItems({"50", "25", "10", "5", "1"});
    cmbDps->setEditable(true);
    cmbDps->setCurrentText("25");
    cmbDps->setStyleSheet(comboStyle);

    // 4. mm/s Combobox
    QLabel *lblMms = new QLabel("mm/s:");
    QComboBox *cmbMms = new QComboBox();
    cmbMms->addItems({"100", "50", "25", "10", "5"});
    cmbMms->setEditable(true);
    cmbMms->setCurrentText("50");
    cmbMms->setStyleSheet(comboStyle);

    unitLayout->addWidget(lblDeg, 0, 0); unitLayout->addWidget(cmbDeg, 0, 1);
    unitLayout->addWidget(lblMm, 1, 0);  unitLayout->addWidget(cmbMm, 1, 1);
    unitLayout->addWidget(lblDps, 0, 2); unitLayout->addWidget(cmbDps, 0, 3);
    unitLayout->addWidget(lblMms, 1, 2); unitLayout->addWidget(cmbMms, 1, 3);
    jogLayout->addWidget(grpUnits);

    // ==========================================
    // ✅ BOTTOM SECTION: J1 TO J6 GRID
    // ==========================================
    QGroupBox *grpJoints = new QGroupBox("Joint Controls");
    grpJoints->setStyleSheet("color: white; font-weight: bold;");
    QGridLayout *jGrid = new QGridLayout(grpJoints);

    for(int i = 1; i <= 6; i++) {
        QLabel *lbl = new QLabel(QString("J%1 :").arg(i));
        QPushButton *btnMinus = new QPushButton("-");
        QPushButton *btnPlus = new QPushButton("+");

        QString btnStyle = "QPushButton { background-color: #3E3E42; color: white; font-size: 16px; font-weight: bold; padding: 5px; min-width: 40px; } "
                           "QPushButton:pressed { background-color: #007ACC; }";
        btnMinus->setStyleSheet(btnStyle);
        btnPlus->setStyleSheet(btnStyle);

        jGrid->addWidget(lbl, i-1, 0);
        jGrid->addWidget(btnMinus, i-1, 1);
        jGrid->addWidget(btnPlus, i-1, 2);

        // 🛠 WIRING: Read BOTH the Step and Speed Comboboxes live!
        connect(btnPlus, &QPushButton::clicked, this, [this, i, rdoMove, cmbDeg, cmbDps]() {
            double step = cmbDeg->currentText().toDouble();
            double speed = cmbDps->currentText().toDouble();
            handleRobotJog(i, 1.0, rdoMove->isChecked(), step, speed);
        });
        connect(btnMinus, &QPushButton::clicked, this, [this, i, rdoMove, cmbDeg, cmbDps]() {
            double step = cmbDeg->currentText().toDouble();
            double speed = cmbDps->currentText().toDouble();
            handleRobotJog(i, -1.0, rdoMove->isChecked(), step, speed);
        });
    }
    jogLayout->addWidget(grpJoints);
    jogLayout->addStretch();

    bottomTabs->addTab(jogTab, "Robot Control");
    rightSplitter->addWidget(bottomTabs);

    rightLayout->addWidget(rightSplitter);
    mainSplitter->addWidget(rightPanel);

    mainSplitter->setSizes(QList<int>({700, 300}));
    rightSplitter->setSizes(QList<int>({500, 500}));

    // ==========================================
    // 2. STATUS BAR & TOOLBAR
    // ==========================================
    QStatusBar *statusBar = new QStatusBar(this);
    setStatusBar(statusBar);
    statusBar->setStyleSheet("background-color: #007ACC; color: #FFFFFF; font-weight: bold; padding-left: 10px;");
    statusBar->showMessage("Ready - 70/30 Layout Active");

    connect(myMainWidget, &OcctWidget::statusUpdate, this, [statusBar](const QString& msg) {
        statusBar->showMessage(msg);
    });

    QToolBar *topBar = new QToolBar("Industrial Controls", this);
    topBar->setMovable(false);
    topBar->setStyleSheet("QToolBar { background-color: #2D2D30; border-bottom: 2px solid #3E3E42; padding: 5px; }");
    addToolBar(Qt::TopToolBarArea, topBar);

    QPushButton *btnLoadModel = new QPushButton("📂 Load Part");
    btnLoadModel->setStyleSheet("QPushButton { background-color: #2E8B57; color: white; font-weight: bold; padding: 6px 12px; margin-right: 5px; }");
    topBar->addWidget(btnLoadModel);

    // ✅ NEW: The Position Part Button
    QPushButton *btnMovePart = new QPushButton("📐 Position Part");
    btnMovePart->setStyleSheet("QPushButton { background-color: #8E24AA; color: white; font-weight: bold; padding: 6px 12px; margin-right: 15px;}");
    topBar->addWidget(btnMovePart);

    QPushButton *btnLoadKuka = new QPushButton("🤖 Load Main Robot");
    btnLoadKuka->setStyleSheet("QPushButton { background-color: #FFA500; color: black; font-weight: bold; padding: 6px 12px; margin-right: 15px;}");
    topBar->addWidget(btnLoadKuka);

    QComboBox *cmbSelectionMode = new QComboBox(this);
    cmbSelectionMode->setStyleSheet("QComboBox { background-color: #3E3E42; color: white; padding: 5px; font-weight: bold; border-radius: 3px; margin-right: 15px; }");
    cmbSelectionMode->addItems({"📦 Select Whole Part", "🔲 Select Face", "📏 Select Edge", "🔗 Select Wire"});
    cmbSelectionMode->setCurrentIndex(1); // Default to Face Selection
    topBar->addWidget(cmbSelectionMode);

    QPushButton *btnClear = new QPushButton("❌ Clear Paths");
    btnClear->setStyleSheet("QPushButton { background-color: #B22222; color: white; font-weight: bold; padding: 6px 12px;}");
    topBar->addWidget(btnClear);

    // ==========================================
    // 3. WIRING IT ALL TOGETHER
    // ==========================================
    connect(btnLoadModel, &QPushButton::clicked, this, &MainWindow::openLoadDialog);
    connect(btnLoadKuka, &QPushButton::clicked, myMainWidget, &OcctWidget::loadDefaultRobot);
    connect(btnClear, &QPushButton::clicked, myMainWidget, &OcctWidget::clearSelections);

    // ✅ NEW: Dialog box logic for Calibration
    connect(btnMovePart, &QPushButton::clicked, this, [this]() {
        QDialog dialog(this);
        dialog.setWindowTitle("Calibrate Workpiece Location");
        dialog.setStyleSheet("background-color: #2D2D30; color: white;");

        QFormLayout form(&dialog);
        QDoubleSpinBox *xBox = new QDoubleSpinBox(&dialog);
        QDoubleSpinBox *yBox = new QDoubleSpinBox(&dialog);
        QDoubleSpinBox *zBox = new QDoubleSpinBox(&dialog);

        // Setup boxes to handle large millimeter distances
        xBox->setRange(-5000, 5000); yBox->setRange(-5000, 5000); zBox->setRange(-5000, 5000);
        xBox->setStyleSheet("background-color: #1E1E1E; color: white; padding: 5px;");
        yBox->setStyleSheet("background-color: #1E1E1E; color: white; padding: 5px;");
        zBox->setStyleSheet("background-color: #1E1E1E; color: white; padding: 5px;");

        form.addRow("X Offset from Robot (mm):", xBox);
        form.addRow("Y Offset from Robot (mm):", yBox);
        form.addRow("Z Offset from Robot (mm):", zBox);

        QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal, &dialog);
        buttonBox.setStyleSheet("QPushButton { background-color: #3E3E42; color: white; padding: 5px 15px;}");
        form.addRow(&buttonBox);
        connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
        connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

        if (dialog.exec() == QDialog::Accepted) {
            // Apply the shift to the MAIN widget
            myMainWidget->offsetWorkpiece(xBox->value(), yBox->value(), zBox->value());
        }
    });

    // Dropdown changes the mode of the RIGHT widget ONLY
    connect(cmbSelectionMode, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [this](int index) {
        mySideWidget->setSelectionMode(index);
    });

    // Left Widget sends Part to Right Widget
    // ... existing connect lines ...
    connect(myMainWidget, &OcctWidget::partSelectedForIsolation, mySideWidget, &OcctWidget::displayIsolatedPart);
    connect(mySideWidget, &OcctWidget::coordinatesExtracted, myCoordinateLog, &QTextEdit::setPlainText);

    // ==========================================
    // ✅ NEW: SAFE AUTO-LOAD TRIGGER
    // ==========================================
    // ✅ SAFE AUTO-LOAD TRIGGER
    // ==========================================
    // Wait 500ms for Linux/X11 to fully build the screen before loading
    QTimer::singleShot(500, this, [this]() {
        myMainWidget->loadDefaultRobot();
    });
    // ==========================================
    // ✅ CRITICAL FIX: INITIALIZE KINEMATICS
    // This builds the KDLJoint arrays so they don't crash when clicked!
    // ==========================================
    kinematic startupKine;
    startupKine.Init();
} // <--- End of MainWindow constructor

MainWindow::~MainWindow() {}

void MainWindow::onPartSelected(const TopoDS_Shape& shape, const QString& xyzData)
{
    mySideWidget->displayIsolatedPart(shape);
    myCoordinateLog->setPlainText(xyzData);
}

void MainWindow::triggerOriginMode() { myMainWidget->enableOriginSelectionMode(); }
void MainWindow::resetOrigin() { myMainWidget->resetOrigin(); }
void MainWindow::loadModel(const std::string& filepath) { myMainWidget->loadStepFile(filepath); }

void MainWindow::openLoadDialog()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open STEP File"), "", tr("STEP Files (*.step *.stp);;All Files (*)"));
    if (!fileName.isEmpty()) {
        myMainWidget->loadStepFile(fileName.toStdString());
    }
}

void MainWindow::handleRobotJog(int jointNumber, double direction, bool isMoveMode, double stepSizeDeg, double speedDps)
{
    if (isMoveMode) {
        // MOVE MODE (Inverse Kinematics)
        qDebug() << "Executing MOVE calculation for J" << jointNumber;

        // TODO: We will write the IK logic here next! For now, we leave it empty.

    } else {
        // JOG MODE (Forward Kinematics)
        qDebug() << "Executing JOG for J" << jointNumber << "| Step:" << stepSizeDeg << "deg | Speed:" << speedDps << "dps";

        // 1. Update the KDL Mathematical Array
        KDLJointCur(jointNumber - 1) += (direction * stepSizeDeg * (M_PI / 180.0));

        kinematic myKine;
        if(myKine.Fk()) {
            qDebug() << "FK Success. Tool -> X:" << cart.p.x() << " Y:" << cart.p.y() << " Z:" << cart.p.z();

            // ==========================================
            // ✅ THE FINAL LINK: Move the 3D Graphics!
            // ==========================================
            myMainWidget->updateRobotPosture(
                KDLJointCur(0), KDLJointCur(1), KDLJointCur(2),
                KDLJointCur(3), KDLJointCur(4), KDLJointCur(5)
                );
        }
    }
}