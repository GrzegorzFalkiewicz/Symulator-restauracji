#include "clientwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHeaderView>

ClientWindow::ClientWindow(QWidget* parent) : QWidget(parent)
{
    socket_ = new QTcpSocket(this);
    setupUi();

    connect(socket_, &QTcpSocket::connected, this, &ClientWindow::onConnected);
    connect(socket_, &QTcpSocket::readyRead, this, &ClientWindow::onReadyRead);
    connect(socket_, &QTcpSocket::disconnected, this, &ClientWindow::onDisconnected);
}

void ClientWindow::setupUi()
{
    setWindowTitle("Zdalny Monitor Restauracji (KLIENT)");
    resize(500, 400);

    auto* root = new QVBoxLayout(this);

    // Connection Header
    auto* header = new QHBoxLayout();
    statusLabel_ = new QLabel("Status: Rozlaczony");
    statusLabel_->setStyleSheet("color: #e06c75; font-weight: bold;");
    connectBtn_ = new QPushButton("Polacz z serwerem");
    
    header->addWidget(statusLabel_);
    header->addStretch();
    header->addWidget(connectBtn_);
    root->addLayout(header);

    // Stats Grid
    auto* statsGroup = new QGroupBox("Dane z sieci (JSON)");
    auto* statsGrid = new QGridLayout(statsGroup);
    
    createdLabel_ = new QLabel("-");
    servedLabel_ = new QLabel("-");
    queueLabel_ = new QLabel("-");

    statsGrid->addWidget(new QLabel("Utworzone:"), 0, 0);
    statsGrid->addWidget(createdLabel_, 0, 1);
    statsGrid->addWidget(new QLabel("Obsluzone:"), 0, 2);
    statsGrid->addWidget(servedLabel_, 0, 3);
    statsGrid->addWidget(new QLabel("W kolejce kuchni:"), 1, 0);
    statsGrid->addWidget(queueLabel_, 1, 1);
    root->addWidget(statsGroup);

    // Cooks Table
    cooksTable_ = new QTableWidget(0, 3);
    cooksTable_->setHorizontalHeaderLabels({"Kucharz", "Stan", "ID"});
    cooksTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    root->addWidget(new QLabel("Stan kucharzy (dane zdalne):"));
    root->addWidget(cooksTable_);

    connect(connectBtn_, &QPushButton::clicked, this, &ClientWindow::toggleConnection);

    // Dark theme for client
    setStyleSheet(R"(
        QWidget { background-color: #21252b; color: #abb2bf; font-size: 10pt; }
        QGroupBox { border: 1px solid #3e4451; margin-top: 10px; font-weight: bold; }
        QPushButton { background-color: #3e4451; padding: 5px; border-radius: 3px; color: white; }
        QTableWidget { background-color: #282c34; gridline-color: #3e4451; }
        QHeaderView::section { background-color: #21252b; }
    )");
}

void ClientWindow::toggleConnection()
{
    if (socket_->state() == QAbstractSocket::UnconnectedState) {
        socket_->connectToHost("127.0.0.1", 12345);
        connectBtn_->setText("Laczenie...");
    } else {
        socket_->disconnectFromHost();
    }
}

void ClientWindow::onConnected()
{
    statusLabel_->setText("Status: POLACZONO");
    statusLabel_->setStyleSheet("color: #98c379; font-weight: bold;");
    connectBtn_->setText("Rozlacz");
}

void ClientWindow::onDisconnected()
{
    statusLabel_->setText("Status: Rozlaczony");
    statusLabel_->setStyleSheet("color: #e06c75; font-weight: bold;");
    connectBtn_->setText("Polacz z serwerem");
}

void ClientWindow::onReadyRead()
{
    QByteArray data = socket_->readAll();
    // TCP streams can contain multiple JSON objects separated by \n
    QList<QByteArray> lines = data.split('\n');
    for (const QByteArray& line : lines) {
        if (line.trimmed().isEmpty()) continue;
        
        QJsonDocument doc = QJsonDocument::fromJson(line);
        if (!doc.isNull() && doc.isObject()) {
            updateDashboard(doc.object());
        }
    }
}

void ClientWindow::updateDashboard(const QJsonObject& data)
{
    createdLabel_->setText(QString::number(data["created"].toInt()));
    servedLabel_->setText(QString::number(data["served"].toInt()));
    queueLabel_->setText(QString::number(data["kitchen"].toInt()));

    QJsonArray cooks = data["cooks"].toArray();
    cooksTable_->setRowCount(cooks.size());
    for (int i = 0; i < cooks.size(); ++i) {
        QJsonObject c = cooks[i].toObject();
        cooksTable_->setItem(i, 0, new QTableWidgetItem(c["name"].toString()));
        cooksTable_->setItem(i, 1, new QTableWidgetItem(c["status"].toString()));
        cooksTable_->setItem(i, 2, new QTableWidgetItem(QString::number(c["orderId"].toInt())));
    }
}
