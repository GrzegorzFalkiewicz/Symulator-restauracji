#include "simulationengine.h"

#include <QDateTime>
#include <QMetaObject>

#include <chrono>
#include <random>
#include <utility>

namespace {
constexpr int CookCount = 3;
constexpr int WaiterCount = 2;
constexpr int MaxLogEntries = 80;

std::mt19937& randomEngine()
{
    thread_local std::mt19937 engine{
        static_cast<unsigned int>(
            std::chrono::high_resolution_clock::now().time_since_epoch().count())};
    return engine;
}
}

SimulationEngine::SimulationEngine(QObject* receiver, SnapshotCallback callback)
    : receiver_(receiver), callback_(std::move(callback))
{
    std::lock_guard<std::mutex> lock(mutex_);
    resetLocked();
}

SimulationEngine::~SimulationEngine()
{
    stop();
}

void SimulationEngine::start()
{
    if (running_.exchange(true)) {
        return;
    }

    {
        std::lock_guard<std::mutex> lock(mutex_);
        stopRequested_ = false;
        for (auto& cook : cooks_) {
            cook.status = "Czeka na zamowienie";
            cook.orderId = 0;
            cook.dish = "";
        }
        for (auto& waiter : waiters_) {
            waiter.status = "Czeka na klienta";
            waiter.orderId = 0;
            waiter.dish = "";
        }
        addLogLocked("Symulacja uruchomiona");
    }

    launchThreads();
    postUpdate();
}

void SimulationEngine::stop()
{
    if (!running_.exchange(false)) {
        return;
    }

    stopRequested_ = true;
    timerCv_.notify_all();
    newOrderCv_.notify_all();
    kitchenCv_.notify_all();

    if (generatorThread_.joinable()) {
        generatorThread_.join();
    }
    for (auto& thread : cookThreads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    for (auto& thread : waiterThreads_) {
        if (thread.joinable()) {
            thread.join();
        }
    }
    cookThreads_.clear();
    waiterThreads_.clear();

    {
        std::lock_guard<std::mutex> lock(mutex_);
        for (auto& cook : cooks_) {
            cook.status = "Zatrzymany";
            cook.orderId = 0;
            cook.dish = "";
        }
        for (auto& waiter : waiters_) {
            waiter.status = "Zatrzymany";
            waiter.orderId = 0;
            waiter.dish = "";
        }
        addLogLocked("Symulacja zatrzymana");
    }
    postUpdate();
}

void SimulationEngine::reset()
{
    stop();
    {
        std::lock_guard<std::mutex> lock(mutex_);
        resetLocked();
        addLogLocked("Symulacja zresetowana");
    }
    postUpdate();
}

void SimulationEngine::setSpeedMultiplier(float multiplier)
{
    speedMultiplier_.store(multiplier);
}

void SimulationEngine::sleepScaled(int ms)
{
    int scaledMs = static_cast<int>(ms / speedMultiplier_.load());
    std::this_thread::sleep_for(std::chrono::milliseconds(scaledMs));
}

void SimulationEngine::resetLocked()
{
    newOrders_ = std::queue<Order>{};
    kitchenOrders_ = std::queue<Order>{};
    readyOrders_ = std::queue<Order>{};
    log_.clear();
    createdOrders_ = 0;
    servedOrders_ = 0;
    nextOrderId_ = 1;

    cooks_.clear();
    waiters_.clear();
    for (int i = 0; i < CookCount; ++i) {
        cooks_.push_back({QString("Kucharz %1").arg(i + 1), "Bezczynny", 0, ""});
    }
    for (int i = 0; i < WaiterCount; ++i) {
        waiters_.push_back({QString("Kelner %1").arg(i + 1), "Bezczynny", 0, ""});
    }
}

void SimulationEngine::launchThreads()
{
    generatorThread_ = std::thread(&SimulationEngine::generatorLoop, this);
    for (int i = 0; i < cooks_.size(); ++i) {
        cookThreads_.emplace_back(&SimulationEngine::cookLoop, this, i);
    }
    for (int i = 0; i < waiters_.size(); ++i) {
        waiterThreads_.emplace_back(&SimulationEngine::waiterLoop, this, i);
    }
}

void SimulationEngine::generatorLoop()
{
    while (!stopRequested_) {
        std::unique_lock<std::mutex> timerLock(mutex_);
        int waitMs = static_cast<int>(900 / speedMultiplier_.load());
        timerCv_.wait_for(timerLock, std::chrono::milliseconds(waitMs), [this] {
            return stopRequested_.load();
        });
        if (stopRequested_) {
            break;
        }

        Order order;
        order.id = nextOrderId_++;
        order.dish = randomDish();
        newOrders_.push(order);
        ++createdOrders_;
        addLogLocked(QString("Klient zlozyl zamowienie #%1: %2").arg(order.id).arg(order.dish));
        timerLock.unlock();

        newOrderCv_.notify_one();
        postUpdate();
    }
}

void SimulationEngine::cookLoop(int index)
{
    while (!stopRequested_) {
        Order order;
        {
            std::unique_lock<std::mutex> lock(mutex_);
            cooks_[index].status = "Czeka na zamowienie";
            cooks_[index].orderId = 0;

            kitchenCv_.wait(lock, [this] {
                return stopRequested_.load() || !kitchenOrders_.empty();
            });
            if (stopRequested_) {
                break;
            }

            order = kitchenOrders_.front();
            kitchenOrders_.pop();
            cooks_[index].status = "Przygotowuje";
            cooks_[index].orderId = order.id;
            cooks_[index].dish = order.dish;
            addLogLocked(QString("Kucharz rozpoczal zamowienie #%1").arg(order.id));
        }
        postUpdate();

        sleepScaled(randomDelayMs(1800, 3600));

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopRequested_) {
                break;
            }
            cooks_[index].status = "Wystawia na lade";
        }
        postUpdate();

        sleepScaled(randomDelayMs(600, 1000));

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopRequested_) {
                break;
            }
            readyOrders_.push(order);
            cooks_[index].status = "Oddal danie";
            cooks_[index].orderId = order.id;
            cooks_[index].dish = order.dish;
            addLogLocked(QString("Gotowe danie dla zamowienia #%1").arg(order.id));
        }
        newOrderCv_.notify_one();
        postUpdate();
    }
}

void SimulationEngine::waiterLoop(int index)
{
    while (!stopRequested_) {
        Order order;
        bool deliveringReadyOrder = false;

        {
            std::unique_lock<std::mutex> lock(mutex_);
            waiters_[index].status = "Czeka na zadanie";
            waiters_[index].orderId = 0;

            newOrderCv_.wait_for(lock, std::chrono::milliseconds(250), [this] {
                return stopRequested_.load() || !readyOrders_.empty() || !newOrders_.empty();
            });
            if (stopRequested_) {
                break;
            }

            if (!readyOrders_.empty()) {
                order = readyOrders_.front();
                readyOrders_.pop();
                deliveringReadyOrder = true;
                waiters_[index].status = "Dostarcza danie";
                waiters_[index].orderId = order.id;
                waiters_[index].dish = order.dish;
                addLogLocked(QString("Kelner odbiera gotowe zamowienie #%1").arg(order.id));
            } else if (!newOrders_.empty()) {
                order = newOrders_.front();
                newOrders_.pop();
                waiters_[index].status = "Przekazuje do kuchni";
                waiters_[index].orderId = order.id;
                waiters_[index].dish = order.dish;
                addLogLocked(QString("Kelner przyjal zamowienie #%1").arg(order.id));
            } else {
                continue;
            }
        }
        postUpdate();

        sleepScaled(deliveringReadyOrder ? randomDelayMs(700, 1300) : randomDelayMs(500, 1100));

        {
            std::lock_guard<std::mutex> lock(mutex_);
            if (stopRequested_) {
                break;
            }

            if (deliveringReadyOrder) {
                ++servedOrders_;
                waiters_[index].status = "Obsluzyl klienta";
                addLogLocked(QString("Klient odebral zamowienie #%1").arg(order.id));
            } else {
                kitchenOrders_.push(order);
                waiters_[index].status = "Zamowienie w kuchni";
                addLogLocked(QString("Zamowienie #%1 trafilo do kolejki kuchni").arg(order.id));
            }
            waiters_[index].orderId = order.id;
        }

        if (!deliveringReadyOrder) {
            kitchenCv_.notify_one();
        }
        postUpdate();
    }
}

void SimulationEngine::postUpdate()
{
    SimulationSnapshot snapshot;
    {
        std::lock_guard<std::mutex> lock(mutex_);
        snapshot = snapshotLocked();
    }

    if (!receiver_ || !callback_) {
        return;
    }

    QMetaObject::invokeMethod(receiver_, [callback = callback_, snapshot] {
        callback(snapshot);
    }, Qt::QueuedConnection);
}

SimulationSnapshot SimulationEngine::snapshotLocked() const
{
    SimulationSnapshot snapshot;
    snapshot.running = running_;
    snapshot.createdOrders = createdOrders_;
    snapshot.waitingOrders = static_cast<int>(newOrders_.size());
    snapshot.kitchenOrders = static_cast<int>(kitchenOrders_.size());
    snapshot.readyOrders = static_cast<int>(readyOrders_.size());
    snapshot.servedOrders = servedOrders_;

    for (const auto& cook : cooks_) {
        snapshot.cooks.push_back({cook.name, cook.status, cook.orderId, cook.dish});
    }
    for (const auto& waiter : waiters_) {
        snapshot.waiters.push_back({waiter.name, waiter.status, waiter.orderId, waiter.dish});
    }
    snapshot.log = log_;
    return snapshot;
}

void SimulationEngine::addLogLocked(const QString& message)
{
    const QString time = QDateTime::currentDateTime().toString("HH:mm:ss");
    log_.prepend(QString("[%1] %2").arg(time, message));
    while (log_.size() > MaxLogEntries) {
        log_.removeLast();
    }
}

QString SimulationEngine::randomDish()
{
    static const QStringList dishes = {
        "Pizza", "Zupa", "Makaron", "Pierogi", "Salatka", "Burger", "Ryba", "Nalesniki"
    };
    std::uniform_int_distribution<int> distribution(0, dishes.size() - 1);
    return dishes.at(distribution(randomEngine()));
}

int SimulationEngine::randomDelayMs(int minMs, int maxMs)
{
    std::uniform_int_distribution<int> distribution(minMs, maxMs);
    return distribution(randomEngine());
}
