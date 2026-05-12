#include "mainwindow.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QVBoxLayout>

MainWindow::MainWindow(QWidget* parent)
    : QWidget(parent),
      engine_(this, [this](const SimulationSnapshot& snapshot) { applySnapshot(snapshot); })
{
    buildUi();
    engine_.reset();
}

void MainWindow::buildUi()
{
    setWindowTitle("Wielowatkowy symulator restauracji");
    resize(980, 640);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(14, 14, 14, 14);
    root->setSpacing(12);

    auto* header = new QHBoxLayout();
    statusLabel_ = new QLabel("Status: bezczynny");
    QFont statusFont = statusLabel_->font();
    statusFont.setPointSize(12);
    statusFont.setBold(true);
    statusLabel_->setFont(statusFont);

    startButton_ = new QPushButton("Start");
    stopButton_ = new QPushButton("Stop");
    resetButton_ = new QPushButton("Reset");
    stopButton_->setEnabled(false);

    header->addWidget(statusLabel_);
    header->addStretch();
    header->addWidget(startButton_);
    header->addWidget(stopButton_);
    header->addWidget(resetButton_);
    root->addLayout(header);

    auto* statsGroup = new QGroupBox("Stan zamowien");
    auto* statsGrid = new QGridLayout(statsGroup);
    createdLabel_ = new QLabel("0");
    waitingLabel_ = new QLabel("0");
    kitchenLabel_ = new QLabel("0");
    readyLabel_ = new QLabel("0");
    servedLabel_ = new QLabel("0");

    statsGrid->addWidget(new QLabel("Utworzone:"), 0, 0);
    statsGrid->addWidget(createdLabel_, 0, 1);
    statsGrid->addWidget(new QLabel("U kelnerow:"), 0, 2);
    statsGrid->addWidget(waitingLabel_, 0, 3);
    statsGrid->addWidget(new QLabel("W kolejce kuchni:"), 1, 0);
    statsGrid->addWidget(kitchenLabel_, 1, 1);
    statsGrid->addWidget(new QLabel("Gotowe:"), 1, 2);
    statsGrid->addWidget(readyLabel_, 1, 3);
    statsGrid->addWidget(new QLabel("Obsluzone:"), 2, 0);
    statsGrid->addWidget(servedLabel_, 2, 1);
    root->addWidget(statsGroup);

    auto* workersLayout = new QHBoxLayout();
    auto* cooksGroup = new QGroupBox("Kucharze");
    auto* cooksLayout = new QVBoxLayout(cooksGroup);
    cooksTable_ = createWorkerTable({"Nazwa", "Stan", "Zamowienie"});
    cooksLayout->addWidget(cooksTable_);

    auto* waitersGroup = new QGroupBox("Kelnerzy");
    auto* waitersLayout = new QVBoxLayout(waitersGroup);
    waitersTable_ = createWorkerTable({"Nazwa", "Stan", "Zamowienie"});
    waitersLayout->addWidget(waitersTable_);

    workersLayout->addWidget(cooksGroup, 3);
    workersLayout->addWidget(waitersGroup, 2);
    root->addLayout(workersLayout, 1);

    auto* logGroup = new QGroupBox("Zdarzenia");
    auto* logLayout = new QVBoxLayout(logGroup);
    logList_ = new QListWidget();
    logLayout->addWidget(logList_);
    root->addWidget(logGroup, 2);

    connect(startButton_, &QPushButton::clicked, this, [this] { engine_.start(); });
    connect(stopButton_, &QPushButton::clicked, this, [this] { engine_.stop(); });
    connect(resetButton_, &QPushButton::clicked, this, [this] { engine_.reset(); });
}

QTableWidget* MainWindow::createWorkerTable(const QStringList& headers)
{
    auto* table = new QTableWidget();
    table->setColumnCount(headers.size());
    table->setHorizontalHeaderLabels(headers);
    table->horizontalHeader()->setStretchLastSection(true);
    table->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    table->verticalHeader()->setVisible(false);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::NoSelection);
    table->setAlternatingRowColors(true);
    return table;
}

void MainWindow::applySnapshot(const SimulationSnapshot& snapshot)
{
    statusLabel_->setText(snapshot.running ? "Status: symulacja dziala" : "Status: zatrzymana");
    createdLabel_->setText(QString::number(snapshot.createdOrders));
    waitingLabel_->setText(QString::number(snapshot.waitingOrders));
    kitchenLabel_->setText(QString::number(snapshot.kitchenOrders));
    readyLabel_->setText(QString::number(snapshot.readyOrders));
    servedLabel_->setText(QString::number(snapshot.servedOrders));

    startButton_->setEnabled(!snapshot.running);
    stopButton_->setEnabled(snapshot.running);
    resetButton_->setEnabled(!snapshot.running);

    fillWorkerTable(cooksTable_, snapshot.cooks);
    fillWorkerTable(waitersTable_, snapshot.waiters);

    logList_->clear();
    for (const auto& entry : snapshot.log) {
        logList_->addItem(entry);
    }
}

void MainWindow::fillWorkerTable(QTableWidget* table, const QVector<WorkerSnapshot>& workers)
{
    table->setRowCount(workers.size());
    for (int row = 0; row < workers.size(); ++row) {
        const auto& worker = workers.at(row);
        table->setItem(row, 0, new QTableWidgetItem(worker.name));
        table->setItem(row, 1, new QTableWidgetItem(worker.status));
        table->setItem(row, 2, new QTableWidgetItem(worker.orderId > 0
            ? QString("#%1").arg(worker.orderId)
            : "-"));
    }
}
