#pragma once

#include <QWidget>
#include <QTcpSocket>
#include <QLabel>
#include <QTextEdit>
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
    QPushButton* connectBtn_;
    QLabel* statusLabel_;
    QLabel* createdLabel_;
    QLabel* servedLabel_;
    QLabel* queueLabel_;
    QTableWidget* cooksTable_;
};
