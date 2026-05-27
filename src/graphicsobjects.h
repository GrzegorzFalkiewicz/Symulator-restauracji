#pragma once

#include <QGraphicsObject>
#include <QPropertyAnimation>

class WorkerItem : public QGraphicsObject {
    Q_OBJECT
    Q_PROPERTY(QPointF pos READ pos WRITE setPos)

public:
    enum class Role { Cook, Waiter };

    explicit WorkerItem(Role role, const QString& name);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    void updateStatus(const QString& status, int orderId, const QString& dish);
    void moveTo(const QPointF& endPos);

private:
    Role role_;
    QString name_;
    QString status_;
    int orderId_ = 0;
    QString dish_;
    QPropertyAnimation* animation_ = nullptr;
};

class StationItem : public QGraphicsItem {
public:
    explicit StationItem(const QString& label, const QColor& color);

    QRectF boundingRect() const override;
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
    QString label_;
    QColor color_;
};
