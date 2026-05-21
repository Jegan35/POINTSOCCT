#include "RightHeader.h"
#include <QHBoxLayout>

RightHeader::RightHeader(QWidget *parent) : QWidget(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(10, 0, 10, 0);
    layout->setSpacing(15);

    // 1. Menu Button
    btnMenu = new QPushButton("≡ MENU", this);
    btnMenu->setStyleSheet("background-color: transparent; color: white; font-size: 20px; font-weight: bold; text-align: left;");
    btnMenu->setCursor(Qt::PointingHandCursor);

    // 2. ✅ NEW: Professional Mechanical Gear Toggle
    btnSwipeToggle = new QPushButton("⚙ SWIPE", this);
    btnSwipeToggle->setCheckable(true); // This makes it act like an ON/OFF switch
    btnSwipeToggle->setCursor(Qt::PointingHandCursor);
    btnSwipeToggle->setStyleSheet(
        "QPushButton { "
        "  background-color: #2D2D30; color: #9CA3AF; "
        "  font-weight: bold; font-size: 13px; padding: 6px 12px; "
        "  border-radius: 4px; border: 1px solid #3E3E42; min-width: 80px; "
        "} "
        "QPushButton:hover { "
        "  background-color: #3E3E42; border: 1px solid #00E5FF; " // Cyan hover glow
        "} "
        "QPushButton:checked { "
        "  background-color: #00E5FF; color: black; border: 1px solid #00E5FF; " // Active state
        "}"
        );

    // 3. The Blinking Bulb
    lblBulb = new QLabel("●", this);
    lblBulb->setStyleSheet("color: #10B981; font-size: 18px;");

    blinkTimer = new QTimer(this);
    connect(blinkTimer, &QTimer::timeout, this, [this]() {
        if (isBulbVisible) {
            lblBulb->setStyleSheet("color: transparent; font-size: 18px;");
        } else {
            lblBulb->setStyleSheet("color: " + bulbColor + "; font-size: 18px;");
        }
        isBulbVisible = !isBulbVisible;
    });
    blinkTimer->start(600);

    // 4. Status Text
    lblStatus = new QLabel("MANUAL | JOG : CARTESIAN", this);
    lblStatus->setStyleSheet("color: #10B981; font-weight: bold; font-size: 14px;");

    QWidget *statusContainer = new QWidget();
    QHBoxLayout *statusLayout = new QHBoxLayout(statusContainer);
    statusLayout->setContentsMargins(0,0,0,0);
    statusLayout->addWidget(lblBulb);
    statusLayout->addWidget(lblStatus);

    // 5. Standby Button
    QPushButton *btnStandby = new QPushButton("STANDBY", this);
    btnStandby->setStyleSheet("background-color: transparent; border: 2px solid #10B981; color: #10B981; font-weight: bold; padding: 5px 15px;");

    // Assemble Layout
    layout->addWidget(btnMenu);
    layout->addWidget(statusContainer, 1, Qt::AlignCenter);
    layout->addWidget(btnSwipeToggle); // Added the new button here
    layout->addWidget(btnStandby);

    // Wire up signals
    connect(btnMenu, &QPushButton::clicked, this, &RightHeader::menuClicked);

    // ✅ Wire the new push button toggle to the swipe lock signal
    connect(btnSwipeToggle, &QPushButton::toggled, this, &RightHeader::swipeLockChanged);
}

void RightHeader::updateStatusText(const QString &text, bool isAutoMode)
{
    lblStatus->setText(text);
    if (isAutoMode) {
        lblStatus->setStyleSheet("color: #F59E0B; font-weight: bold; font-size: 14px;");
        bulbColor = "#F59E0B"; // Orange
    } else {
        lblStatus->setStyleSheet("color: #10B981; font-weight: bold; font-size: 14px;");
        bulbColor = "#10B981"; // Green
    }
}