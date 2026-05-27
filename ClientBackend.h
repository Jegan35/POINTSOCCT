#ifndef CLIENTBACKEND_H
#define CLIENTBACKEND_H

#include <QObject>
#include <QWebSocket>
#include <QJsonObject>
#include <QJsonDocument>
#include <QJsonArray>
#include <QVariantList>
#include <QStringList>
#include <QTimer> // <--- ADDED TIMER
#include "LineGeometry.h"

class ClientBackend : public QObject
{
    Q_OBJECT

    // --- CONNECTION & AUTH ---
    Q_PROPERTY(QString targetIp MEMBER m_targetIp NOTIFY connectionChanged)
    Q_PROPERTY(QString authId MEMBER m_authId NOTIFY connectionChanged)
    Q_PROPERTY(bool isWebClientConnected MEMBER m_isWebClientConnected NOTIFY connectionChanged)

    // --- INDUSTRIAL BATTERY & SAFETY LOCKOUT (NEW) ---
    Q_PROPERTY(int batteryLevel READ batteryLevel NOTIFY batteryLevelChanged)
    Q_PROPERTY(bool isBatteryLow READ isBatteryLow NOTIFY isBatteryLowChanged)
    Q_PROPERTY(bool isCharging READ isCharging NOTIFY isChargingChanged)
    Q_PROPERTY(bool powerLockout READ powerLockout NOTIFY powerLockoutChanged)

    // --- TELEMETRY DATA (Fast Updates) ---
    Q_PROPERTY(QString currentMode MEMBER m_currentMode NOTIFY telemetryChanged)
    Q_PROPERTY(bool isStarted MEMBER m_isStarted NOTIFY telemetryChanged)
    Q_PROPERTY(bool isRunning MEMBER m_isRunning NOTIFY telemetryChanged)
    Q_PROPERTY(bool isServoOn MEMBER m_isServoOn NOTIFY telemetryChanged)
    Q_PROPERTY(QString errorMessage MEMBER m_errorMessage NOTIFY telemetryChanged)
    Q_PROPERTY(bool isCalculatingTrajectory MEMBER m_isCalculatingTrajectory NOTIFY telemetryChanged)
    Q_PROPERTY(bool isPhysicallyMoving MEMBER m_isPhysicallyMoving NOTIFY telemetryChanged)
    Q_PROPERTY(int digitalInputVal MEMBER m_digitalInputVal NOTIFY telemetryChanged)
    Q_PROPERTY(int digitalOutputVal MEMBER m_digitalOutputVal NOTIFY telemetryChanged)

    Q_PROPERTY(double x MEMBER m_x NOTIFY telemetryChanged)
    Q_PROPERTY(double y MEMBER m_y NOTIFY telemetryChanged)
    Q_PROPERTY(double z MEMBER m_z NOTIFY telemetryChanged)
    Q_PROPERTY(double a MEMBER m_a NOTIFY telemetryChanged)
    Q_PROPERTY(double b MEMBER m_b NOTIFY telemetryChanged)
    Q_PROPERTY(double c MEMBER m_c NOTIFY telemetryChanged)
    Q_PROPERTY(double j1 MEMBER m_j1 NOTIFY telemetryChanged)
    Q_PROPERTY(double j2 MEMBER m_j2 NOTIFY telemetryChanged)
    Q_PROPERTY(double j3 MEMBER m_j3 NOTIFY telemetryChanged)
    Q_PROPERTY(double j4 MEMBER m_j4 NOTIFY telemetryChanged)
    Q_PROPERTY(double j5 MEMBER m_j5 NOTIFY telemetryChanged)
    Q_PROPERTY(double j6 MEMBER m_j6 NOTIFY telemetryChanged)

    // Derived State Properties
    Q_PROPERTY(bool isAuto MEMBER m_isAuto NOTIFY telemetryChanged)
    Q_PROPERTY(bool isManual MEMBER m_isManual NOTIFY telemetryChanged)
    Q_PROPERTY(bool isRemote MEMBER m_isRemote NOTIFY telemetryChanged)
    Q_PROPERTY(bool isEmergency MEMBER m_isEmergency NOTIFY telemetryChanged)

    // --- PROGRAM DATA ---
    Q_PROPERTY(QVariantList tpPointData MEMBER m_tpPointData NOTIFY programDataChanged)
    Q_PROPERTY(QVariantList prProgramData MEMBER m_prProgramData NOTIFY programDataChanged)
    Q_PROPERTY(QString currentInstructionString MEMBER m_currentInstructionString NOTIFY programDataChanged)
    Q_PROPERTY(int highlightedInstruction MEMBER m_highlightedInstruction NOTIFY highlightedInstructionChanged)

    Q_PROPERTY(QString stagingName1 MEMBER m_stagingName1 NOTIFY programDataChanged)
    Q_PROPERTY(QString stagingValue1 MEMBER m_stagingValue1 NOTIFY programDataChanged)
    Q_PROPERTY(QString stagingDeg1 MEMBER m_stagingDeg1 NOTIFY programDataChanged)
    Q_PROPERTY(QString stagingName2 MEMBER m_stagingName2 NOTIFY programDataChanged)
    Q_PROPERTY(QString stagingValue2 MEMBER m_stagingValue2 NOTIFY programDataChanged)
    Q_PROPERTY(QString stagingDeg2 MEMBER m_stagingDeg2 NOTIFY programDataChanged)
    Q_PROPERTY(QString stagingSpeed MEMBER m_stagingSpeed NOTIFY programDataChanged)
    Q_PROPERTY(QString stagingComment MEMBER m_stagingComment NOTIFY programDataChanged)

    // --- DIRECTORY DATA ---
    Q_PROPERTY(QStringList tpFileList MEMBER m_tpFileList NOTIFY directoryDataChanged)
    Q_PROPERTY(QStringList prFileList MEMBER m_prFileList NOTIFY directoryDataChanged)
    Q_PROPERTY(QString currentTpName MEMBER m_currentTpName NOTIFY directoryDataChanged)
    Q_PROPERTY(QString currentPrName MEMBER m_currentPrName NOTIFY directoryDataChanged)
    Q_PROPERTY(QString tpRunModeName MEMBER m_tpRunModeName NOTIFY directoryDataChanged)
    Q_PROPERTY(double speedOp MEMBER m_speedOp NOTIFY directoryDataChanged)
    Q_PROPERTY(QString programCountOutput MEMBER m_programCountOutput NOTIFY directoryDataChanged)
    Q_PROPERTY(QJsonObject robotSettings MEMBER m_robotSettings NOTIFY robotSettingsChanged)

    // --- UI/LOCAL STATE ---
    Q_PROPERTY(bool isMoveMode MEMBER m_isMoveMode NOTIFY localStateChanged)
    Q_PROPERTY(QString variableOutputValue MEMBER m_variableOutputValue NOTIFY localStateChanged)

    // --- GEOMETRY POINTERS ---
    Q_PROPERTY(LineGeometry* blueLine READ blueLine WRITE setBlueLine NOTIFY blueLineChanged)
    Q_PROPERTY(LineGeometry* redLine READ redLine WRITE setRedLine NOTIFY redLineChanged)

    Q_PROPERTY(QVariantList encoderRawValues MEMBER m_encoderRawValues NOTIFY encoderDataChanged)
    Q_PROPERTY(QVariantList encoderOffsetValues MEMBER m_encoderOffsetValues NOTIFY encoderDataChanged)

public:
    explicit ClientBackend(QObject *parent = nullptr);

    LineGeometry* blueLine() const { return m_blueLine; }
    void setBlueLine(LineGeometry* line) { m_blueLine = line; emit blueLineChanged(); }

    LineGeometry* redLine() const { return m_redLine; }
    void setRedLine(LineGeometry* line) { m_redLine = line; emit redLineChanged(); }

    // --- BATTERY GETTERS ---
    int batteryLevel() const { return m_batteryLevel; }
    bool isBatteryLow() const { return m_isBatteryLow; }
    bool isCharging() const { return m_isCharging; }
    bool powerLockout() const { return m_powerLockout; }

    // --- CORE INVOKABLES ---
    Q_INVOKABLE void connectAndLogin(QString ip, QString role, QString user, QString pass);
    Q_INVOKABLE void disconnectWebClient();
    Q_INVOKABLE void sendCmd(QString cmd, QVariant val = QVariant());
    Q_INVOKABLE void sendTextMessage(const QString &msg);

    Q_INVOKABLE void requestRobotSettings() { sendCmd("GET_ROBOT_SETTINGS"); }
    Q_INVOKABLE void updateRobotSettings(QJsonObject settings);

    // --- SPECIFIC COMMANDS ---
    Q_INVOKABLE void toggleServo() { sendCmd("TOGGLE_SERVO"); }
    Q_INVOKABLE void triggerHome() { sendCmd("TRIGGER_HOME"); }
    Q_INVOKABLE void toggleRunPause() { sendCmd("TOGGLE_PAUSE"); }
    Q_INVOKABLE void toggleStartStop() { sendCmd("TOGGLE_START"); }
    Q_INVOKABLE void triggerExit() { sendCmd("EXIT"); }
    Q_INVOKABLE void clearErrors() { sendCmd("CLEAR_ERRORS"); }
    Q_INVOKABLE void clearMarks() { sendCmd("CLEAR_MARKS"); }
    Q_INVOKABLE void toggleEmergencyStop() { sendCmd("TOGGLE_ESTOP"); }
    Q_INVOKABLE void closeShma() { sendCmd("CLOSE_SHMA"); }

    Q_INVOKABLE void setSimMode() { sendCmd("SET_SIM"); }
    Q_INVOKABLE void setRealMode() { sendCmd("SET_REAL"); }
    Q_INVOKABLE void setAutoMode() { sendCmd("SET_AUTO"); }
    Q_INVOKABLE void setManualMode() { sendCmd("SET_MANUAL"); }
    Q_INVOKABLE void setRemoteMode() { sendCmd("SET_REMOTE"); }
    Q_INVOKABLE void setJogMode() { sendCmd("SET_JOG_MODE"); }
    Q_INVOKABLE void setMoveMode() { sendCmd("SET_MOVE_MODE"); }

    Q_INVOKABLE void handleButtonPress(QString btn) { sendCmd("BTN_PRESS", btn); }
    Q_INVOKABLE void handleButtonRelease(QString btn) { sendCmd("BTN_RELEASE", btn); }
    Q_INVOKABLE void handleButtonClick(QString btn) { sendCmd("BTN_CLICK", btn); }
    Q_INVOKABLE void setMmSpeed(QVariant val) { sendCmd("SET_MM_SPEED", val.toString()); }
    Q_INVOKABLE void setDegSpeed(QVariant val) { sendCmd("SET_DEG_SPEED", val.toString()); }
    Q_INVOKABLE void setStartVelocity(QVariant val) { sendCmd("SET_START_VEL", val.toString()); }
    Q_INVOKABLE void setEndVelocity(QVariant val) { sendCmd("SET_END_VEL", val.toString()); }
    Q_INVOKABLE void setGlobalSpeed(QVariant val) { sendCmd("SET_GLOBAL_SPEED", val.toString()); }
    Q_INVOKABLE void setMmIncrement(QVariant val) { sendCmd("SET_MM_INC", val.toString()); }
    Q_INVOKABLE void setDegIncrement(QVariant val) { sendCmd("SET_DEG_INC", val.toString()); }
    Q_INVOKABLE void setFrame(QString val) { sendCmd("SET_FRAME", val); }

    Q_INVOKABLE void refreshTpFiles() { sendCmd("REFRESH_TP_FILES"); }
    Q_INVOKABLE void handleNewTp(QString name) { sendCmd("NEW_TP_FILE", name); }
    Q_INVOKABLE void handleOpenTp(QString name) { sendCmd("OPEN_TP_FILE", name); }
    Q_INVOKABLE void handleDeleteTp(QString name) { sendCmd("DELETE_TP_FILE", name); }
    Q_INVOKABLE void insertTpPoint() { sendCmd("INSERT_TP"); }
    Q_INVOKABLE void deleteTpPoint(QVariant index) { sendCmd("DELETE_TP_INDEX", index.toString()); }
    Q_INVOKABLE void setTpPointName(QString val) { sendCmd("SET_TP_NAME", val); }
    Q_INVOKABLE void selectTpPoint(QVariant index) { sendCmd("SELECT_TP_INDEX", index.toString()); }
    Q_INVOKABLE void selectTpMidPoint(QVariant index) { sendCmd("SELECT_TP_MID_INDEX", index.toString()); }
    Q_INVOKABLE void runTpPoint() { sendCmd("RUN_TP"); }
    Q_INVOKABLE void setTpRunMode(QString mode) { sendCmd("SET_TP_RUN_MODE", mode); }
    Q_INVOKABLE void handleModifyTp(int id, QString name, QString x, QString y, QString z, QString a, QString b, QString c, QString j1, QString j2, QString j3, QString j4, QString j5, QString j6, QString mode);


    Q_INVOKABLE void handleModifyPr(int id, QString inst, QString speed, QString comment);
    Q_INVOKABLE void refreshPrFiles() { sendCmd("REFRESH_PR_FILES"); }
    Q_INVOKABLE void handleNewPr(QString name) { sendCmd("NEW_PR_FILE", name); }
    Q_INVOKABLE void handleOpenPr(QString name) { sendCmd("OPEN_PR_FILE", name); }
    Q_INVOKABLE void handleDeletePr(QString name) { sendCmd("DELETE_PR_FILE", name); }
    Q_INVOKABLE void insertProgramInstructionAt(QVariant index) { sendCmd("INSERT_PR_INSTRUCTION_AT", index.toString()); }
    Q_INVOKABLE void deleteProgramInstruction() { sendCmd("DELETE_PR_INSTRUCTION"); }
    Q_INVOKABLE void selectProgramRow(QVariant index) { sendCmd("SELECT_PR_ROW", index.toString()); }
    Q_INVOKABLE void setInstructionType(QString type) { sendCmd("SET_INSTRUCTION_TYPE", type); }
    Q_INVOKABLE void setProgramSpeed(QString val) { sendCmd("SET_PROGRAM_SPEED", val); }
    Q_INVOKABLE void setProgramComment(QString val) { sendCmd("SET_PROGRAM_COMMENT", val); }

    Q_INVOKABLE void setGoToProgram(QString val) { sendCmd("SET_GOTO_PROGRAM", val); }
    Q_INVOKABLE void setLoop(QString val) { sendCmd("SET_LOOP", val); }
    Q_INVOKABLE void setDelay(QString val) { sendCmd("SET_DELAY", val); }
    Q_INVOKABLE void setVariable1(QString val) { sendCmd("SET_VAR1", val); }
    Q_INVOKABLE void setVariableValue(QString val) { sendCmd("SET_VAR_VAL", val); }
    Q_INVOKABLE void setVariable2(QString val) { sendCmd("SET_VAR2", val); }
    Q_INVOKABLE void runProgram() { sendCmd("RUN_PROGRAM"); }
    Q_INVOKABLE void setProgramInput(QVariant val) { sendCmd("SET_PROGRAM_INPUT", val.toString()); }

    Q_INVOKABLE void setDigi1(QString val) { sendCmd("SET_DIGI_1", val); }
    Q_INVOKABLE void setDigi2(QString val) { sendCmd("SET_DIGI_2", val); }
    Q_INVOKABLE void setHighLow(QString val) { sendCmd("SET_HIGH_LOW", val); }
    Q_INVOKABLE void confirmHighLow() { sendCmd("CONFIRM_HIGH_LOW"); }
    Q_INVOKABLE void setSimDiNumber(QString val) { sendCmd("SET_SIM_DI_NUMBER", val); }
    Q_INVOKABLE void setSimDiState(QString val) { sendCmd("SET_SIM_DI_STATE", val); }
    Q_INVOKABLE void setSimDoNumber(QString val) { sendCmd("SET_SIM_DO_NUMBER", val); }
    Q_INVOKABLE void setSimDoState(QString val) { sendCmd("SET_SIM_DO_STATE", val); }

    Q_INVOKABLE void setVariableInputSelector(QString val) { sendCmd("SET_VAR_IN_SEL", val); }
    Q_INVOKABLE void setVariableInputValue(QString val) { sendCmd("SET_VAR_IN_VAL", val); }
    Q_INVOKABLE void setVariableInstructionNumber(QString val) { sendCmd("SET_VAR_INS_NUM", val); }
    Q_INVOKABLE void setVariableOutputSelector(QString val) { sendCmd("SET_VAR_OUT_SEL", val); }

    Q_INVOKABLE void calculateTrajectory() { sendCmd("CALCULATE_TRAJECTORY"); }
    Q_INVOKABLE void cancelCalculation() { sendCmd("CANCEL_CALCULATION"); }
    Q_INVOKABLE void triggerEncoderZero() { sendCmd("TRIGGER_ENC_ZERO"); }

signals:
    void connectionChanged();
    void telemetryChanged();
    void programDataChanged();
    void directoryDataChanged();
    void localStateChanged();
    void blueLineChanged();
    void redLineChanged();

    // --- BATTERY SIGNALS ---
    void batteryLevelChanged();
    void isBatteryLowChanged();
    void isChargingChanged();
    void powerLockoutChanged();

    // Routing Signals to QML
    void loginAccepted(QString role);
    void loginRejected(QString message);
    void connectionClosed();
    void userListResult(QString roleType, QVariantList users);
    void userModificationResult();
    void highlightedInstructionChanged(); // ADD THIS NEW SIGNAL
    void robotSettingsChanged();
    void encoderDataChanged();

private slots:
    void onConnected();
    void onDisconnected();
    void onTextMessageReceived(const QString &message);
    void checkAndroidBattery(); // <--- ADDED SLOT

private:
    QWebSocket *m_socket;

    // Login Cache
    QString m_pendingRole;
    QString m_pendingUser;
    QString m_pendingPass;

    // Member Variables
    QString m_targetIp;
    QString m_authId;
    bool m_isWebClientConnected = false;

    bool m_isAuto = false;
    bool m_isManual = true; // Defaults to true
    bool m_isRemote = false;
    bool m_isEmergency = false;

    // Battery Variables
    QTimer *m_batteryTimer;
    int m_batteryLevel = 100;
    bool m_isBatteryLow = false;
    bool m_hasWarnedUser = false;
    bool m_isCharging = false;
    bool m_powerLockout = false;

    QString m_currentMode = "Sim";
    bool m_isStarted = false;
    bool m_isRunning = false;
    bool m_isServoOn = false;
    QString m_errorMessage = "No Active Errors";
    bool m_isCalculatingTrajectory = false;
    bool m_isPhysicallyMoving = false;
    int m_digitalInputVal = 0;
    int m_digitalOutputVal = 0;

    bool m_isMoveMode = false;
    QString m_variableOutputValue = "0";

    double m_x = 0, m_y = 0, m_z = 0, m_a = 0, m_b = 0, m_c = 0;
    double m_j1 = 0, m_j2 = 0, m_j3 = 0, m_j4 = 0, m_j5 = 0, m_j6 = 0;

    QVariantList m_tpPointData;
    QVariantList m_prProgramData;
    QString m_currentInstructionString;
    int m_highlightedInstruction = -1;

    QString m_stagingName1 = "--", m_stagingValue1 = "--", m_stagingDeg1 = "--";
    QString m_stagingName2 = "--", m_stagingValue2 = "--", m_stagingDeg2 = "--";
    QString m_stagingSpeed = "", m_stagingComment = "";

    QStringList m_tpFileList;
    QStringList m_prFileList;
    QString m_currentTpName = "None";
    QString m_currentPrName = "None";
    QString m_tpRunModeName = "TP Mode";
    double m_speedOp = 0.0;
    QString m_programCountOutput = "0";

    LineGeometry *m_blueLine = nullptr;
    LineGeometry *m_redLine = nullptr;
    QJsonObject m_robotSettings;

    QVariantList m_encoderRawValues;
    QVariantList m_encoderOffsetValues;
};

#endif // CLIENTBACKEND_H