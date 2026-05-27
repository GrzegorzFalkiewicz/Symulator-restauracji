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
    resize(700, 650);

    auto* root = new QVBoxLayout(this);

    // Connection Header
    auto* header = new QHBoxLayout();
    statusLabel_ = new QLabel("Status: Rozlaczony");
    statusLabel_->setStyleSheet("color: #e06c75; font-weight: bold;");
    
    header->addWidget(statusLabel_);
    header->addStretch();
    
    header->addWidget(new QLabel("IP:"));
    ipInput_ = new QLineEdit("127.0.0.1");
    ipInput_->setFixedWidth(120);
    header->addWidget(ipInput_);

    connectBtn_ = new QPushButton("Polacz");
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
    root->addWidget(statsGroup, 0); // No stretch for stats

    // Cooks Table
    auto* cooksGroup = new QGroupBox("Stan kucharzy");
    auto* cooksLayout = new QVBoxLayout(cooksGroup);
    cooksTable_ = new QTableWidget(0, 4);
    cooksTable_->setHorizontalHeaderLabels({"Kucharz", "Stan", "ID", "Danie"});
    cooksTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    cooksTable_->setMinimumHeight(150);
    cooksLayout->addWidget(cooksTable_);
    root->addWidget(cooksGroup, 1); // Give stretch factor 1

    // Waiters Table
    auto* waitersGroup = new QGroupBox("Stan kelnerow");
    auto* waitersLayout = new QVBoxLayout(waitersGroup);
    waitersTable_ = new QTableWidget(0, 4);
    waitersTable_->setHorizontalHeaderLabels({"Kelner", "Stan", "ID", "Danie"});
    waitersTable_->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    waitersTable_->setMinimumHeight(150);
    waitersLayout->addWidget(waitersTable_);
    root->addWidget(waitersGroup, 1); // Give stretch factor 1

    connect(connectBtn_, &QPushButton::clicked, this, &ClientWindow::toggleConnection);

    // Dark theme for client
    setStyleSheet(R"(
        QWidget { background-color: #21252b; color: #abb2bf; font-size: 10pt; }
        QGroupBox { border: 1px solid #3e4451; margin-top: 10px; font-weight: bold; color: #61afef; }
        QPushButton { background-color: #3e4451; padding: 5px; border-radius: 3px; color: white; min-width: 70px; }
        QPushButton:hover { background-color: #4b5263; }
        QLineEdit { background-color: #282c34; border: 1px solid #3e4451; padding: 3px; color: #d7dae0; border-radius: 2px; }
        QTableWidget { background-color: #282c34; gridline-color: #3e4451; border: none; }
        QHeaderView::section { background-color: #21252b; border: 1px solid #3e4451; padding: 4px; color: #abb2bf; }
    )");
}

void ClientWindow::toggleConnection()
{
    if (socket_->state() == QAbstractSocket::UnconnectedState) {
        QString ip = ipInput_->text().trimmed();
        if (ip.isEmpty()) ip = "127.0.0.1";
        socket_->connectToHost(ip, 12345);
        connectBtn_->setText("Laczenie...");
        ipInput_->setEnabled(false);
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
    connectBtn_->setText("Polacz");
    ipInput_->setEnabled(true);
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

    // Helper for tables
    auto updateTable = [](QTableWidget* table, const QJsonArray& arr) {
        table->setRowCount(arr.size());
        for (int i = 0; i < arr.size(); ++i) {
            QJsonObject o = arr[i].toObject();
            table->setItem(i, 0, new QTableWidgetItem(o["name"].toString()));
            table->setItem(i, 1, new QTableWidgetItem(o["status"].toString()));
            int id = o["orderId"].toInt();
            table->setItem(i, 2, new QTableWidgetItem(id > 0 ? QString("#%1").arg(id) : "-"));
            table->setItem(i, 3, new QTableWidgetItem(o["dish"].toString()));
        }
    };

    updateTable(cooksTable_, data["cooks"].toArray());
    updateTable(waitersTable_, data["waiters"].toArray());
}
