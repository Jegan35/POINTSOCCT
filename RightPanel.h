#ifndef RIGHTPANEL_H
#define RIGHTPANEL_H

#include <QWidget>
#include <QStackedWidget>
#include <QTabWidget>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QComboBox>
#include <QLineEdit>
#include <QVBoxLayout>
#include <QElapsedTimer>
#include "ClientBackend.h"
#include "RightHeader.h"
#include "MenuTray.h"

class OcctWidget;
class QTextEdit;

class RightPanel : public QWidget
{
    Q_OBJECT

public:
    OcctWidget* getDxfPreviewWidget() const { return m_dxfPreviewWidget; }
    explicit RightPanel(ClientBackend *backend, QWidget *parent = nullptr);
    void setActiveTab(int index);

public slots:
    void setGetPointsEnabled(bool enabled);

signals:
    void swipeLockToggled(bool isUnlocked);
    void requestFooterSwipe();
    void maximizedToggled(bool isMaximized);

protected:
    void resizeEvent(QResizeEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onTelemetryChanged();
    void onProgramDataChanged();
    void onDirectoryDataChanged();
    void onLocalStateChanged();
    void onHighlightChanged();

private:
    OcctWidget *m_dxfPreviewWidget = nullptr;
    QTextEdit *m_txtCoordinates = nullptr;
    void setupUI();
    void toggleMaximized();
    void updateIOLeds();
    void updateInstructionTable();
    void updatePrTable();
    void updateTpTable();
    void updateOpPgDisplay();

    // ✅ ADDED: Dialog popups
    void showModifyTpDialog(int row);
    void showModifyPrDialog(int row);

    // Widget builders
    QWidget* buildSpeedPanel();
    QWidget* buildJointsPanel();
    QWidget* buildIOModulesWidget();
    QWidget* buildDataVarWidget();
    QWidget* buildAxisLimitWidget();
    QWidget* buildPrTableWidget();
    QWidget* buildTpTableWidget();
    QWidget* buildInstructionTableWidget();
    QWidget* buildTpCtrlWidget();
    QWidget* buildPrCtrlWidget();
    QWidget* buildDxfFileWidget();

    // ---- Core ----
    ClientBackend *m_backend;
    QVBoxLayout   *m_mainLayout    = nullptr;
    bool           m_isMaximized   = false;

    // ---- Normal-mode sections ----
    RightHeader    *header       = nullptr;
    QStackedWidget *controlStack = nullptr;
    MenuTray       *tray         = nullptr;

    // ---- Jog controls (cartesian page) ----
    QPushButton *btnToggleXYZ   = nullptr;
    QPushButton *btnToggleOrient = nullptr;
    QPushButton *btnXPlus = nullptr, *btnXMinus = nullptr;
    QPushButton *btnYPlus = nullptr, *btnYMinus = nullptr;
    QPushButton *btnZPlus = nullptr, *btnZMinus = nullptr;
    QPushButton *m_btnGetPoints;
    QPushButton *m_btnRunDxf; // Add this to your private variables in RightPanel.h
    QString currentMovementMode = "JOG";

    // ---- Workspace tabs (shared, always present) ----
    QTabWidget  *m_workspaceTabs = nullptr;
    QPushButton *m_btnMax        = nullptr;

    // ---- Max-mode extra sections ----
    QWidget        *m_instructionTableWidget = nullptr;
    QStackedWidget *m_controlSwipeStack      = nullptr;

    // ---- PR table ----
    QTableWidget *m_prTable = nullptr;

    // ---- TP table ----
    QTableWidget *m_tpTable = nullptr;

    // ---- IO LEDs ----
    QLabel *m_diLeds[16] = {};
    QLabel *m_doLeds[16] = {};

    // ---- Instruction (staging) row cells ----
    QLabel *m_stgInst    = nullptr;
    QLabel *m_stgName1   = nullptr, *m_stgVal1 = nullptr, *m_stgDeg1 = nullptr;
    QLabel *m_stgName2   = nullptr, *m_stgVal2 = nullptr, *m_stgDeg2 = nullptr;
    QLabel *m_stgSpeed   = nullptr, *m_stgComment = nullptr;

    // ---- Data-Var tab ----
    QComboBox *m_varOutSel  = nullptr;
    QLabel    *m_varOutVal  = nullptr;
    QComboBox *m_varInSel   = nullptr;
    QLineEdit *m_varInInput = nullptr;
    QLineEdit *m_varInstNum = nullptr;

    // ---- Axis-Limit / Simulation tab ----
    QComboBox *m_simDiNum   = nullptr;
    QComboBox *m_simDiState = nullptr;
    QComboBox *m_simDoNum   = nullptr;
    QComboBox *m_simDoState = nullptr;

    // ---- TP-CTRL panel ----
    QPushButton *m_btnTpModeLabel = nullptr;
    QString      m_tpModeDisplay  = "TP Mode";
    QLabel      *m_lblOpPg        = nullptr;
    QComboBox   *m_tpParamSel     = nullptr;
    QLineEdit   *m_tpParamVal     = nullptr;

    // ---- PR-CTRL panel ----
    QComboBox *m_instSel    = nullptr;
    QComboBox *m_di1Sel     = nullptr;
    QComboBox *m_di2Sel     = nullptr;
    QComboBox *m_hlSel      = nullptr;
    QComboBox *m_var1Sel    = nullptr;
    QComboBox *m_var2Sel    = nullptr;
    QComboBox *m_prParamSel = nullptr;
    QLineEdit *m_prParamVal = nullptr;

    // ---- Swipe tracking (control area) ----
    QPoint m_ctrlSwipeStart;
    bool   m_ctrlSwipeTracking = false;

    // ---- UI throttle ----
    QElapsedTimer m_uiThrottle;
};

#endif // RIGHTPANEL_H