#pragma once

#include "simulationengine.h"

#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QTableWidget>
#include <QWidget>

class RestaurantView;

class MainWindow : public QWidget {
public:
    explicit MainWindow(QWidget* parent = nullptr);
    ~MainWindow() override = default;

private:
    void buildUi();
    void applySnapshot(const SimulationSnapshot& snapshot);
    QTableWidget* createWorkerTable(const QStringList& headers);
    void fillWorkerTable(QTableWidget* table, const QVector<WorkerSnapshot>& workers);

    SimulationEngine engine_;

    RestaurantView* restaurantView_ = nullptr;
    QLabel* statusLabel_ = nullptr;
    QLabel* createdLabel_ = nullptr;
    QLabel* waitingLabel_ = nullptr;
    QLabel* kitchenLabel_ = nullptr;
    QLabel* readyLabel_ = nullptr;
    QLabel* servedLabel_ = nullptr;

    QPushButton* startButton_ = nullptr;
    QPushButton* stopButton_ = nullptr;
    QPushButton* resetButton_ = nullptr;

    QTableWidget* cooksTable_ = nullptr;
    QTableWidget* waitersTable_ = nullptr;
    QListWidget* logList_ = nullptr;
};
