#include "ClientBackend.h"
#include <QDebug>

// --- THE FIX IS HERE: Use QCoreApplication instead of QNativeInterface ---
#ifdef Q_OS_ANDROID
#include <QJniObject>
#include <QCoreApplication>
#endif

ClientBackend::ClientBackend(QObject *parent) : QObject(parent)
{
    m_socket = new QWebSocket();
    connect(m_socket, &QWebSocket::connected, this, &ClientBackend::onConnected);
    connect(m_socket, &QWebSocket::disconnected, this, &ClientBackend::onDisconnected);
    connect(m_socket, &QWebSocket::textMessageReceived, this, &ClientBackend::onTextMessageReceived);

    // --- INITIALIZE BATTERY MONITORING ---
    m_batteryTimer = new QTimer(this);
    connect(m_batteryTimer, &QTimer::timeout, this, &ClientBackend::checkAndroidBattery);
    m_batteryTimer->start(60000); // Check every 60 seconds

    checkAndroidBattery(); // Run initial check
}

void ClientBackend::connectAndLogin(QString ip, QString role, QString user, QString pass)
{
    m_targetIp = ip;
    m_pendingRole = role;
    m_pendingUser = user;
    m_pendingPass = pass;
    emit connectionChanged();

    m_socket->close();
    m_socket->open(QUrl("ws://" + ip + ":8080"));
}

void ClientBackend::disconnectWebClient()
{
    m_socket->close();
    m_isWebClientConnected = false;
    emit connectionChanged();
}

void ClientBackend::onConnected()
{
    QJsonObject loginReq;
    loginReq["command"] = "LOGIN";
    loginReq["role"] = m_pendingRole;
    loginReq["user"] = m_pendingUser;
    loginReq["pass"] = m_pendingPass;
    loginReq["hw_id"] = m_authId;
    m_socket->sendTextMessage(QJsonDocument(loginReq).toJson(QJsonDocument::Compact));
}

void ClientBackend::onDisconnected()
{
    m_isWebClientConnected = false;
    emit connectionChanged();
    emit connectionClosed();
}

void ClientBackend::sendCmd(QString cmd, QVariant val)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject req;
        req["command"] = cmd;
        req["value"] = val.toString();
        m_socket->sendTextMessage(QJsonDocument(req).toJson(QJsonDocument::Compact));
    }
}

void ClientBackend::handleModifyTp(int id, QString name, QString x, QString y, QString z, QString a, QString b, QString c, QString j1, QString j2, QString j3, QString j4, QString j5, QString j6, QString mode)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject data;
        data["id"] = id; data["name"] = name;
        data["x"] = x; data["y"] = y; data["z"] = z;
        data["a"] = a; data["b"] = b; data["c"] = c;
        data["j1"] = j1; data["j2"] = j2; data["j3"] = j3;
        data["j4"] = j4; data["j5"] = j5; data["j6"] = j6;
        data["mode"] = mode; // "CART" or "JOINT"

        QJsonObject req;
        req["command"] = "MODIFY_TP";
        req["data"] = data;
        m_socket->sendTextMessage(QJsonDocument(req).toJson(QJsonDocument::Compact));
    }
}
void ClientBackend::handleModifyPr(int id, QString inst, QString speed, QString comment)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject data;
        data["id"] = id;
        data["inst"] = inst;
        data["speed"] = speed;
        data["comment"] = comment;

        QJsonObject req;
        req["command"] = "MODIFY_PR";
        req["data"] = data;
        m_socket->sendTextMessage(QJsonDocument(req).toJson(QJsonDocument::Compact));
    }
}

// Add this anywhere in ClientBackend.cpp
void ClientBackend::sendTextMessage(const QString &msg)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->sendTextMessage(msg);
    }
}
void ClientBackend::updateRobotSettings(QJsonObject settings) {
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        QJsonObject req;
        req["command"] = "UPDATE_ROBOT_SETTINGS";
        req["data"] = settings;
        m_socket->sendTextMessage(QJsonDocument(req).toJson(QJsonDocument::Compact));
    }
}
void ClientBackend::onTextMessageReceived(const QString &message)
{
    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    QJsonObject json = doc.object();
    QString type = json["type"].toString();

    // --- AUTHENTICATION ---
    if (type == "login_accepted") {
        m_isWebClientConnected = true;
        emit connectionChanged();
        emit loginAccepted(json["role"].toString());
    }
    else if (type == "login_rejected" || type == "force_disconnect") {
        m_isWebClientConnected = false;
        m_socket->close();
        emit connectionChanged();
        emit loginRejected(json["message"].toString());
    }
    else if (type == "user_list_result") {
        emit userListResult(json["role_type"].toString(), json["users"].toArray().toVariantList());
    }
    else if (type == "user_creation_result" || type == "user_delete_result" || type == "user_modify_result") {
        emit userModificationResult();
    }
    else if (type == "robot_settings_data") {
        m_robotSettings = json["data"].toObject();
        emit robotSettingsChanged();
    }
    // --- SPLIT-STREAM DATA ---
    else if (type == "telemetry") {
        QJsonObject cart = json["cartesian"].toObject();
        m_x = cart["x"].toDouble(); m_y = cart["y"].toDouble(); m_z = cart["z"].toDouble();
        m_a = cart["rx"].toDouble(); m_b = cart["ry"].toDouble(); m_c = cart["rz"].toDouble();

        QJsonObject j = json["joints"].toObject();
        m_j1 = j["j1"].toDouble(); m_j2 = j["j2"].toDouble(); m_j3 = j["j3"].toDouble();
        m_j4 = j["j4"].toDouble(); m_j5 = j["j5"].toDouble(); m_j6 = j["j6"].toDouble();

        QString rawMode = json["mode"].toString();
        m_currentMode = rawMode.split("_").first(); // Keeps "Sim" or "Real" for your UI
        m_isAuto = rawMode.contains("Auto");
        m_isManual = rawMode.contains("Manual");
        m_isRemote = rawMode.contains("Remote");
        m_isEmergency = rawMode.contains("Emergency");
        m_isStarted = json["started"].toBool();
        m_isRunning = json["paused"].toBool();
        m_isServoOn = json["servo_on"].toBool();
        m_errorMessage = json["error_message"].toString();
        m_isCalculatingTrajectory = json["is_calculating_trajectory"].toBool();
        m_isPhysicallyMoving = json["is_physically_moving"].toBool();
        m_digitalInputVal = json["di_val"].toInt();
        m_digitalOutputVal = json["do_val"].toInt();
        QJsonArray rawArray = json["encoder_raw"].toArray();
        QJsonArray offsetArray = json["encoder_offset"].toArray();

        m_encoderRawValues.clear();
        m_encoderOffsetValues.clear();

        for(const QJsonValue& val : rawArray) {
            m_encoderRawValues.append(val.toDouble());
        }

        for(const QJsonValue& val : offsetArray) {
            m_encoderOffsetValues.append(val.toDouble());
        }

        emit encoderDataChanged();

        emit telemetryChanged(); // Batches all updates to QML at once!
    }
    else if (type == "program_data") {
        m_tpPointData = json["tp_list"].toArray().toVariantList();
        m_prProgramData = json["pr_program_data"].toArray().toVariantList();

        QJsonObject staging = json["staging_data"].toObject();
        m_currentInstructionString = staging["instruction"].toString();
        m_stagingName1 = staging["name1"].toString();
        m_stagingValue1 = staging["value1"].toString();
        m_stagingDeg1 = staging["deg1"].toString();
        m_stagingName2 = staging["name2"].toString();
        m_stagingValue2 = staging["value2"].toString();
        m_stagingDeg2 = staging["deg2"].toString();
        m_stagingSpeed = staging["speed"].toString();
        m_stagingComment = staging["comment"].toString();

        m_highlightedInstruction = json["highlighted_instruction"].isUndefined() ? -1 : json["highlighted_instruction"].toInt();

        emit highlightedInstructionChanged(); // Emit this too so it catches the initial load
        emit programDataChanged();
    }
    // --- ADD THIS NEW BLOCK ---
    else if (type == "highlight_update") {
        m_highlightedInstruction = json["highlighted_instruction"].toInt();
        emit highlightedInstructionChanged(); // ONLY updates the highlight, leaves the QML list intact!
    }
    else if (type == "directory_data") {
        m_tpFileList.clear();
        for (QJsonValue val : json["tp_file_list"].toArray()) m_tpFileList.append(val.toString());

        m_prFileList.clear();
        for (QJsonValue val : json["pr_file_list"].toArray()) m_prFileList.append(val.toString());

        m_currentTpName = json["current_tp_name"].toString();
        m_currentPrName = json["current_pr_name"].toString();
        m_tpRunModeName = json["tp_run_mode"].toString();
        m_speedOp = json["speed_op"].toDouble();
        m_programCountOutput = json["program_count_output"].toString();

        emit directoryDataChanged();
    }
    else if (type == "trajectory_chunk") {
        QString color = json["color"].toString();

        QByteArray rawBytes = QByteArray::fromBase64(json["points_base64"].toString().toLatin1());
        const float *ptr = reinterpret_cast<const float*>(rawBytes.constData());
        int floatCount = rawBytes.size() / sizeof(float);

        QVariantList pts;
        pts.reserve(floatCount);
        for(int i = 0; i < floatCount; ++i) {
            pts.append(ptr[i]);
        }

        if (color == "blue" && m_blueLine) m_blueLine->addPointsList(pts);
        else if (color == "red" && m_redLine) m_redLine->addPointsList(pts);
    }
    else if (type == "clear_trajectories") {
        if (m_blueLine) m_blueLine->clear();
        if (m_redLine) m_redLine->clear();
    }
}

void ClientBackend::checkAndroidBattery()
{
#ifdef Q_OS_ANDROID
    QJniObject context = QCoreApplication::instance()->property("androidContext").value<QJniObject>();
    if (!context.isValid()) {
        context = QNativeInterface::QAndroidApplication::context();
    }

    if (context.isValid()) {
        QJniObject intentFilter("android/content/IntentFilter", "(Ljava/lang/String;)V",
                                QJniObject::fromString("android.intent.action.BATTERY_CHANGED").object<jstring>());

        QJniObject intent = context.callObjectMethod("registerReceiver",
                                                     "(Landroid/content/BroadcastReceiver;Landroid/content/IntentFilter;)Landroid/content/Intent;",
                                                     nullptr, intentFilter.object());

        if (intent.isValid()) {
            // 1. Get Battery Level
            int level = intent.callMethod<jint>("getIntExtra", "(Ljava/lang/String;I)I",
                                                QJniObject::fromString("level").object<jstring>(), -1);
            int scale = intent.callMethod<jint>("getIntExtra", "(Ljava/lang/String;I)I",
                                                QJniObject::fromString("scale").object<jstring>(), -1);

            // 2. Get Charging Status (2 = Charging, 5 = Full)
            int status = intent.callMethod<jint>("getIntExtra", "(Ljava/lang/String;I)I",
                                                 QJniObject::fromString("status").object<jstring>(), -1);

            if (level != -1 && scale != -1) {
                int currentPercentage = (int)(((float)level / (float)scale) * 100.0f);

                if (m_batteryLevel != currentPercentage) {
                    m_batteryLevel = currentPercentage;
                    emit batteryLevelChanged();
                }

                // Evaluate Charging State
                bool currentlyCharging = (status == 2 || status == 5);
                if (m_isCharging != currentlyCharging) {
                    m_isCharging = currentlyCharging;
                    emit isChargingChanged();
                }

                // --- INDUSTRIAL FAIL-SAFE LOGIC ---
                // Lockout if battery is below 20% AND the charger is NOT plugged in
                bool shouldLock = (m_batteryLevel < 20 && !m_isCharging);

                if (m_powerLockout != shouldLock) {
                    m_powerLockout = shouldLock;
                    emit powerLockoutChanged();

                    // Optional Safety Interlock: Automatically pause the robot if the UI locks out
                    if (m_powerLockout && m_isRunning) {
                        sendCmd("TOGGLE_PAUSE");
                    }
                }
            }
        }
    }
#endif
}