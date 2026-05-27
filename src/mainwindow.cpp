#include "mainwindow.h"
#include "restaurantview.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QGridLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QHBoxLayout>
#include <QSlider>
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
    resize(1100, 800);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(14, 14, 14, 14);
    root->setSpacing(12);

    auto* header = new QHBoxLayout();
    statusLabel_ = new QLabel("Status: bezczynny");
    statusLabel_->setObjectName("statusLabel");
    QFont statusFont = statusLabel_->font();
    statusFont.setPointSize(12);
    statusFont.setBold(true);
    statusLabel_->setFont(statusFont);

    // Speed Control
    auto* speedLayout = new QHBoxLayout();
    speedLayout->setContentsMargins(20, 0, 20, 0);
    speedLayout->addWidget(new QLabel("Predkosc:"));
    speedSlider_ = new QSlider(Qt::Horizontal);
    speedSlider_->setRange(5, 50); // 0.5x to 5.0x
    speedSlider_->setValue(10);    // 1.0x
    speedSlider_->setFixedWidth(150);
    speedValueLabel_ = new QLabel("1.0x");
    speedValueLabel_->setFixedWidth(40);
    speedLayout->addWidget(speedSlider_);
    speedLayout->addWidget(speedValueLabel_);

    startButton_ = new QPushButton("Start");
    stopButton_ = new QPushButton("Stop");
    resetButton_ = new QPushButton("Reset");
    stopButton_->setEnabled(false);

    header->addWidget(statusLabel_);
    header->addStretch();
    header->addLayout(speedLayout);
    header->addWidget(startButton_);
    header->addWidget(stopButton_);
    header->addWidget(resetButton_);
    root->addLayout(header);

    // Visualisation
    auto* visualGroup = new QGroupBox("Podglad restauracji");
    auto* visualLayout = new QVBoxLayout(visualGroup);
    restaurantView_ = new RestaurantView();
    visualLayout->addWidget(restaurantView_);
    root->addWidget(visualGroup, 4);

    auto* statsGroup = new QGroupBox("Stan zamowien");
    auto* statsGrid = new QGridLayout(statsGroup);
    
    auto createStatLabel = [this, statsGrid](const QString& text, QLabel*& valLabel, int r, int c) {
        statsGrid->addWidget(new QLabel(text), r, c);
        valLabel = new QLabel("0");
        valLabel->setObjectName("statValue");
        statsGrid->addWidget(valLabel, r, c + 1);
    };

    createStatLabel("Utworzone:", createdLabel_, 0, 0);
    createStatLabel("U kelnerow:", waitingLabel_, 0, 2);
    createStatLabel("W kolejce kuchni:", kitchenLabel_, 1, 0);
    createStatLabel("Gotowe:", readyLabel_, 1, 2);
    createStatLabel("Obsluzone:", servedLabel_, 2, 0);
    
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
    
    auto* logGroup = new QGroupBox("Zdarzenia");
    auto* logLayout = new QVBoxLayout(logGroup);
    logList_ = new QListWidget();
    logLayout->addWidget(logList_);
    
    auto* bottomLayout = new QHBoxLayout();
    bottomLayout->addLayout(workersLayout, 3);
    bottomLayout->addWidget(logGroup, 2);
    root->addLayout(bottomLayout, 3);

    connect(startButton_, &QPushButton::clicked, this, [this] { engine_.start(); });
    connect(stopButton_, &QPushButton::clicked, this, [this] { engine_.stop(); });
    connect(resetButton_, &QPushButton::clicked, this, [this] { engine_.reset(); });

    connect(speedSlider_, &QSlider::valueChanged, this, [this](int value) {
        float multiplier = value / 10.0f;
        speedValueLabel_->setText(QString("%1x").arg(multiplier, 0, 'f', 1));
        engine_.setSpeedMultiplier(multiplier);
        restaurantView_->setSpeedMultiplier(multiplier);
    });
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

    // Optimized log update: only update if count or top entry changed
    if (logList_->count() != snapshot.log.size() || (snapshot.log.size() > 0 && logList_->item(0)->text() != snapshot.log.at(0))) {
        logList_->setUpdatesEnabled(false);
        logList_->clear();
        for (const auto& entry : snapshot.log) {
            logList_->addItem(entry);
        }
        logList_->setUpdatesEnabled(true);
    }

    restaurantView_->updateSnapshot(snapshot);
}

void MainWindow::fillWorkerTable(QTableWidget* table, const QVector<WorkerSnapshot>& workers)
{
    if (table->rowCount() != workers.size()) {
        table->setRowCount(workers.size());
    }
    
    for (int row = 0; row < workers.size(); ++row) {
        const auto& worker = workers.at(row);
        
        auto setItemText = [&](int col, const QString& text) {
            QTableWidgetItem* item = table->item(row, col);
            if (!item) {
                item = new QTableWidgetItem(text);
                table->setItem(row, col, item);
            } else if (item->text() != text) {
                item->setText(text);
            }
        };

        setItemText(0, worker.name);
        setItemText(1, worker.status);
        setItemText(2, worker.orderId > 0 ? QString("#%1").arg(worker.orderId) : "-");
    }
}
