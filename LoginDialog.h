#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QButtonGroup>
#include "ClientBackend.h"

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(ClientBackend *backend, QWidget *parent = nullptr);

signals:
    void accepted(); // Add this

private slots:
    void handleLoginClicked();
    void handleDesignModeClicked();
    void onLoginAccepted(QString role);
    void onLoginRejected(QString message);
    void onConnectionClosed();

private:
    ClientBackend *m_backend;

    QLineEdit *ipInput;
    QLineEdit *userInput;
    QLineEdit *passInput;

    QButtonGroup *roleGroup;
    QLabel *errorLabel;
    QPushButton *btnLogin;
    QPushButton *btnDesignMode;

    QString getSelectedRole();
};

#endif // LOGINDIALOG_H