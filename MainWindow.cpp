#include "MainWindow.h"
#include "LoginDialog.h"
#include <QApplication>
#include <QStackedWidget>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    m_backend = new ClientBackend(this);

    QStackedWidget *mainStack = new QStackedWidget(this);
    setCentralWidget(mainStack);

    LoginDialog *loginPage = new LoginDialog(m_backend, mainStack);
    mainStack->addWidget(loginPage);

    QWidget *dashboardPage = new QWidget(mainStack);
    dashboardPage->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    QVBoxLayout *dashLayout = new QVBoxLayout(dashboardPage);
    dashLayout->setContentsMargins(0, 0, 0, 0);
    dashLayout->setSpacing(0);
    mainStack->addWidget(dashboardPage);

    connect(loginPage, &LoginDialog::loginSuccessful, this,
            [this, mainStack, dashboardPage, dashLayout]() {

                // 1. Swap page immediately
                mainStack->setCurrentIndex(1);

                // 2. Build the Splitter
                this->mainSplitter = new QSplitter(Qt::Horizontal, dashboardPage);
                this->mainSplitter->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                this->mainSplitter->setStyleSheet(
                    "QSplitter::handle { background-color: #1E1E1E; width: 4px; }");

                // 3. Build Panels
                this->leftPanel  = new LeftPanel (this->m_backend, this->mainSplitter);
                this->rightPanel = new RightPanel(this->m_backend, this->mainSplitter);

                this->leftPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
                this->rightPanel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

                this->mainSplitter->addWidget(this->leftPanel);
                this->mainSplitter->addWidget(this->rightPanel);

                // 4. Force 50/50 Layout Proportionately
                this->mainSplitter->setStretchFactor(0, 1);
                this->mainSplitter->setStretchFactor(1, 1);
                this->mainSplitter->setCollapsible(0, false);
                this->mainSplitter->setCollapsible(1, false);

                dashLayout->addWidget(this->mainSplitter, 1);

                // 5. Wire the Panels together
                connect(this->leftPanel->getMainOcctWidget(),  &OcctWidget::partSelectedForIsolation,
                        this->rightPanel->getDxfPreviewWidget(), &OcctWidget::displayIsolatedPart);

                connect(this->leftPanel,  &LeftPanel::partSelectionStateChanged,
                        this->rightPanel, &RightPanel::setGetPointsEnabled);

                connect(this->rightPanel, &RightPanel::swipeLockToggled,
                        this->leftPanel,  &LeftPanel::setSwipeEnabled);

                connect(this->rightPanel, &RightPanel::requestFooterSwipe,
                        this->leftPanel,  &LeftPanel::toggleFooterSwipe);

                connect(this->leftPanel,  &LeftPanel::requestTabChange,
                        this->rightPanel, &RightPanel::setActiveTab);

                // 6. Connect the "MAX" button toggle to cleanly refresh the layout
                // Make sure `maximizedToggled(bool)` is declared in RightPanel.h!
                connect(this->rightPanel, &RightPanel::maximizedToggled,
                        this, [this](bool) {
                            QList<int> currentSizes = this->mainSplitter->sizes();
                            int total = currentSizes[0] + currentSizes[1];
                            this->mainSplitter->setSizes({total / 2, total / 2});
                        });
            });

    this->setWindowFlags(Qt::Window | Qt::FramelessWindowHint);
    this->showMaximized();
}

MainWindow::~MainWindow() {}