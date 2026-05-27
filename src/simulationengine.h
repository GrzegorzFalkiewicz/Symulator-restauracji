#pragma once

#include "order.h"

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

struct WorkerSnapshot {
    QString name;
    QString status;
    int orderId = 0;
    QString dish;
};

struct SimulationSnapshot {
    bool running = false;
    int createdOrders = 0;
    int waitingOrders = 0;
    int kitchenOrders = 0;
    int readyOrders = 0;
    int servedOrders = 0;
    QVector<WorkerSnapshot> cooks;
    QVector<WorkerSnapshot> waiters;
    QStringList log;
};

class SimulationEngine {
public:
    using SnapshotCallback = std::function<void(const SimulationSnapshot&)>;

    explicit SimulationEngine(QObject* receiver, SnapshotCallback callback);
    ~SimulationEngine();

    void start();
    void stop();
    void reset();

private:
    struct WorkerState {
        QString name;
        QString status = "Bezczynny";
        int orderId = 0;
        QString dish;
    };

    void resetLocked();
    void launchThreads();
    void generatorLoop();
    void cookLoop(int index);
    void waiterLoop(int index);

    void postUpdate();
    SimulationSnapshot snapshotLocked() const;
    void addLogLocked(const QString& message);
    QString randomDish();
    int randomDelayMs(int minMs, int maxMs);

    QObject* receiver_;
    SnapshotCallback callback_;

    mutable std::mutex mutex_;
    std::condition_variable newOrderCv_;
    std::condition_variable kitchenCv_;
    std::condition_variable timerCv_;

    std::queue<Order> newOrders_;
    std::queue<Order> kitchenOrders_;
    std::queue<Order> readyOrders_;

    QVector<WorkerState> cooks_;
    QVector<WorkerState> waiters_;
    QStringList log_;

    std::atomic_bool running_{false};
    std::atomic_bool stopRequested_{false};
    std::thread generatorThread_;
    std::vector<std::thread> cookThreads_;
    std::vector<std::thread> waiterThreads_;

    int nextOrderId_ = 1;
    int createdOrders_ = 0;
    int servedOrders_ = 0;
};
