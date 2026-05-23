#include "MainWindow.h"
#include "LoginDialog.h"
#include <QApplication>
#include <QTimer>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // 1. Create the backend FIRST
    m_backend = new ClientBackend(this);

    // ==========================================================
    // ✅ THE FIX: Stop Qt from killing the app when Login closes
    // ==========================================================
    qApp->setQuitOnLastWindowClosed(false);

    // 2. SHOW THE LOGIN DIALOG FIRST
    LoginDialog login(m_backend, this);
    if (login.exec() != QDialog::Accepted) {
        // If the user presses ALT+F4 or closes the popup, restore rule and quit.
        qApp->setQuitOnLastWindowClosed(true);
        QTimer::singleShot(0, qApp, &QCoreApplication::quit);
        return;
    }

    // 3. Create the main Splitter
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->setStyleSheet("QSplitter::handle { background-color: #1E1E1E; width: 4px; }");
    setCentralWidget(mainSplitter);

    // 4. Instantiate the isolated panels
    leftPanel = new LeftPanel(m_backend, this);
    rightPanel = new RightPanel(m_backend, this);

    // ==========================================================
    // STRICT 50/50 LAYOUT ENFORCEMENT
    // ==========================================================
    leftPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    rightPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);

    connect(rightPanel, &RightPanel::swipeLockToggled, leftPanel, &LeftPanel::setSwipeEnabled);
    connect(rightPanel, &RightPanel::requestFooterSwipe, leftPanel, &LeftPanel::toggleFooterSwipe);
    connect(leftPanel, &LeftPanel::requestTabChange, rightPanel, &RightPanel::setActiveTab);

    mainSplitter->setStretchFactor(0, 1);
    mainSplitter->setStretchFactor(1, 1);
    mainSplitter->setSizes({10000, 10000});

    // 5. Show the window
    this->showMaximized();

    // ==========================================================
    // ✅ THE FIX: Turn the normal quit rule back on now that Main is visible
    // ==========================================================
    qApp->setQuitOnLastWindowClosed(true);
}

MainWindow::~MainWindow()
{
    // Ensure destructor is empty (or only contains actual memory cleanup)
}