#pragma once

#include "graphicsobjects.h"
#include "simulationengine.h"

#include <QGraphicsView>
#include <QMap>

class RestaurantView : public QGraphicsView {
    Q_OBJECT

public:
    explicit RestaurantView(QWidget* parent = nullptr);

    void updateSnapshot(const SimulationSnapshot& snapshot);
    void setSpeedMultiplier(float multiplier);

protected:
    void resizeEvent(QResizeEvent* event) override;

private:
    void setupScene();
    void updateWorkers(const QVector<WorkerSnapshot>& snapshots, WorkerItem::Role role);
    QPointF getWorkerTargetPos(WorkerItem::Role role, const QString& status, int index);

    QGraphicsScene* scene_;
    QMap<QString, WorkerItem*> workers_;
    StationItem* kitchenStation_;
    StationItem* counterStation_;
    StationItem* tableStation_;
    float currentMultiplier_ = 1.0f;
};
