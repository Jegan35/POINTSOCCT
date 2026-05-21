#ifndef RIGHTHEADER_H
#define RIGHTHEADER_H

#include <QWidget>
#include <QPushButton>
#include <QLabel>
#include <QTimer>

class RightHeader : public QWidget
{
    Q_OBJECT

public:
    explicit RightHeader(QWidget *parent = nullptr);
    void updateStatusText(const QString &text, bool isAutoMode = false);

signals:
    void menuClicked();
    void swipeLockChanged(bool isUnlocked);

private:
    QPushButton *btnMenu;
    QPushButton *btnSwipeToggle; // ✅ Changed from QCheckBox to QPushButton
    QLabel *lblStatus;

    // Bulb Animation
    QLabel *lblBulb;
    QTimer *blinkTimer;
    bool isBulbVisible = true;
    QString bulbColor = "#10B981";
};

#endif // RIGHTHEADER_H