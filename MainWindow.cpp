#include "MainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // 1. Create the main 50/50 Splitter
    mainSplitter = new QSplitter(Qt::Horizontal, this);
    mainSplitter->setStyleSheet("QSplitter::handle { background-color: #1E1E1E; width: 4px; }");
    setCentralWidget(mainSplitter);

    // 2. Instantiate the isolated files
    leftPanel = new LeftPanel(this);
    rightPanel = new RightPanel(this);

    // 3. Add to splitter
    mainSplitter->addWidget(leftPanel);
    mainSplitter->addWidget(rightPanel);

    // 4. Force an exact 50/50 split
    mainSplitter->setSizes(QList<int>({INT_MAX, INT_MAX}));

    // Optional: Connect signals from RightPanel (Jog buttons) to LeftPanel (OCCT 3D) here
    // connect(rightPanel, &RightPanel::jogRequested, leftPanel, &LeftPanel::handleJog);
}

MainWindow::~MainWindow() {}