#include "restaurantview.h"

#include <QGraphicsRectItem>

RestaurantView::RestaurantView(QWidget* parent)
    : QGraphicsView(parent)
{
    scene_ = new QGraphicsScene(this);
    setScene(scene_);
    setRenderHint(QPainter::Antialiasing);
    setViewportUpdateMode(QGraphicsView::FullViewportUpdate);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setBackgroundBrush(QColor("#1e1e1e"));

    setupScene();
}

void RestaurantView::setupScene()
{
    scene_->clear();
    workers_.clear();

    // Define Zones
    kitchenStation_ = new StationItem("Kuchnia", QColor("#e06c75"));
    counterStation_ = new StationItem("Lada / Odbior", QColor("#98c379"));
    tableStation_ = new StationItem("Sala / Stoliki", QColor("#61afef"));

    scene_->addItem(kitchenStation_);
    scene_->addItem(counterStation_);
    scene_->addItem(tableStation_);

    kitchenStation_->setPos(100, 150);
    counterStation_->setPos(400, 150);
    tableStation_->setPos(700, 150);

    // Decorative floor
    auto* floor = scene_->addRect(0, 0, 800, 300, QPen(QColor("#333333")), QBrush(QColor("#252525")));
    floor->setZValue(-10);
    scene_->setSceneRect(0, 0, 800, 300);
}

void RestaurantView::updateSnapshot(const SimulationSnapshot& snapshot)
{
    updateWorkers(snapshot.cooks, WorkerItem::Role::Cook);
    updateWorkers(snapshot.waiters, WorkerItem::Role::Waiter);
}

void RestaurantView::updateWorkers(const QVector<WorkerSnapshot>& snapshots, WorkerItem::Role role)
{
    for (int i = 0; i < snapshots.size(); ++i) {
        const auto& snap = snapshots.at(i);
        WorkerItem* item = nullptr;

        if (!workers_.contains(snap.name)) {
            item = new WorkerItem(role, snap.name);
            scene_->addItem(item);
            workers_[snap.name] = item;
            
            // Initial position
            QPointF startPos = getWorkerTargetPos(role, snap.status, i);
            item->setPos(startPos);
        } else {
            item = workers_[snap.name];
        }

        item->updateStatus(snap.status, snap.orderId, snap.dish);
        item->moveTo(getWorkerTargetPos(role, snap.status, i));
    }
}

QPointF RestaurantView::getWorkerTargetPos(WorkerItem::Role role, const QString& status, int index)
{
    if (role == WorkerItem::Role::Cook) {
        if (status.contains("lade") || status.contains("Oddal")) {
            return QPointF(250 + (index * 30), 150); // Near counter from kitchen side
        }
        // Cooks usually stay in kitchen
        return QPointF(50 + index * 40, 100 + (index % 2) * 100);
    } else {
        // Waiters move
        if (status.contains("Przekazuje") || status.contains("odbiera")) {
            return QPointF(380 + (index * 30), 150); // Near counter from waiter side
        } else if (status.contains("Dostarcza") || status.contains("Obsluzyl") || status.contains("klienta")) {
            return QPointF(750, 100 + index * 100); // At tables
        } else {
            return QPointF(450 + (index * 30), 220); // Waiting area
        }
    }
}

void RestaurantView::resizeEvent(QResizeEvent* event)
{
    QGraphicsView::resizeEvent(event);
    fitInView(scene_->sceneRect(), Qt::KeepAspectRatio);
}
