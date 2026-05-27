#pragma once

#include <QWidget>
#include <QTcpSocket>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QTableWidget>

class ClientWindow : public QWidget {
    Q_OBJECT

public:
    explicit ClientWindow(QWidget* parent = nullptr);

private slots:
    void onConnected();
    void onReadyRead();
    void onDisconnected();
    void toggleConnection();

private:
    void setupUi();
    void updateDashboard(const QJsonObject& data);

    QTcpSocket* socket_;
    
    // UI Elements
    QLineEdit* ipInput_;
    QPushButton* connectBtn_;
    QLabel* statusLabel_;
    QLabel* createdLabel_;
    QLabel* servedLabel_;
    QLabel* queueLabel_;
    QTableWidget* cooksTable_;
    QTableWidget* waitersTable_;
};
