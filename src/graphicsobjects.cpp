#include "graphicsobjects.h"

#include <QPainter>
#include <QStyleOptionGraphicsItem>

WorkerItem::WorkerItem(Role role, const QString& name)
    : role_(role), name_(name)
{
    animation_ = new QPropertyAnimation(this, "pos", this);
    animation_->setDuration(600);
    animation_->setEasingCurve(QEasingCurve::InOutQuad);
}

QRectF WorkerItem::boundingRect() const
{
    return QRectF(-30, -30, 60, 80);
}

void WorkerItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    // Body
    QColor color = (role_ == Role::Cook) ? QColor("#e06c75") : QColor("#61afef");
    if (status_ == "Bezczynny" || status_ == "Zatrzymany") {
        color = color.darker(150);
    }

    painter->setBrush(color);
    painter->setPen(Qt::NoPen);
    painter->drawEllipse(-20, -20, 40, 40);

    // Hat / Detail
    painter->setBrush(Qt::white);
    if (role_ == Role::Cook) {
        painter->drawRect(-10, -28, 20, 10); // Chef hat
    } else {
        painter->drawRect(-15, -18, 30, 4); // Waiter tie/apron detail
    }

    // Name
    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setPointSize(8);
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(QRectF(-30, 25, 60, 15), Qt::AlignCenter, name_);

    // Status / Dish
    font.setBold(false);
    font.setPointSize(7);
    painter->setFont(font);
    painter->setPen(QColor("#abb2bf"));
    QString detail = status_;
    if (orderId_ > 0) {
        detail = QString("#%1 %2").arg(orderId_).arg(dish_);
    }
    painter->drawText(QRectF(-50, 40, 100, 30), Qt::AlignCenter | Qt::TextWordWrap, detail);
}

void WorkerItem::updateStatus(const QString& status, int orderId, const QString& dish)
{
    status_ = status;
    orderId_ = orderId;
    dish_ = dish;
    update();
}

void WorkerItem::moveTo(const QPointF& endPos)
{
    if (pos() == endPos) return;
    animation_->stop();
    animation_->setStartValue(pos());
    animation_->setEndValue(endPos);
    animation_->start();
}

StationItem::StationItem(const QString& label, const QColor& color)
    : label_(label), color_(color)
{
}

QRectF StationItem::boundingRect() const
{
    return QRectF(-60, -40, 120, 80);
}

void StationItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setRenderHint(QPainter::Antialiasing);

    QLinearGradient grad(0, -40, 0, 40);
    grad.setColorAt(0, color_.lighter(120));
    grad.setColorAt(1, color_.darker(120));

    painter->setBrush(grad);
    painter->setPen(QPen(color_.lighter(150), 2));
    painter->drawRoundedRect(-60, -40, 120, 80, 10, 10);

    painter->setPen(Qt::white);
    QFont font = painter->font();
    font.setPointSize(10);
    font.setBold(true);
    painter->setFont(font);
    painter->drawText(boundingRect(), Qt::AlignCenter, label_);
}
