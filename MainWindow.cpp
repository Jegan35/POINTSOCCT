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

    // Bottom 50% of Right Panel: XYZ Text Output
    myCoordinateLog = new QTextEdit(this);
    myCoordinateLog->setReadOnly(true);
    myCoordinateLog->setStyleSheet("background-color: #1E1E1E; color: #00FF00; font-family: monospace;");
    rightSplitter->addWidget(myCoordinateLog);

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