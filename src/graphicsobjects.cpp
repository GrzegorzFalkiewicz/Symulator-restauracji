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

    QColor baseColor = (role_ == Role::Cook) ? QColor("#e06c75") : QColor("#61afef");
    if (status_ == "Bezczynny" || status_ == "Zatrzymany") {
        baseColor = baseColor.darker(150);
    }

    // --- Draw Body Icon ---
    painter->setPen(Qt::NoPen);
    
    // Body (trapezoid/rounded rect for shoulders)
    painter->setBrush(baseColor);
    painter->drawRoundedRect(-15, -5, 30, 25, 5, 5);
    
    // Head
    painter->drawEllipse(-10, -22, 20, 20);

    // --- Role Specific Details ---
    if (role_ == Role::Cook) {
        // Chef Hat
        painter->setBrush(Qt::white);
        painter->setPen(QPen(Qt::lightGray, 0.5));
        painter->drawRoundedRect(-12, -35, 24, 15, 3, 3);
        painter->drawEllipse(-14, -40, 10, 10);
        painter->drawEllipse(-5, -42, 10, 10);
        painter->drawEllipse(4, -40, 10, 10);
    } else {
        // Waiter Bowtie
        painter->setBrush(QColor("#282c34"));
        QPainterPath bowtie;
        bowtie.moveTo(-6, -2);
        bowtie.lineTo(6, 4);
        bowtie.lineTo(6, -2);
        bowtie.lineTo(-6, 4);
        bowtie.closeSubpath();
        painter->drawPath(bowtie);
        
        // Shirt front (white triangle)
        painter->setBrush(Qt::white);
        QPolygonF shirt;
        shirt << QPointF(-4, -5) << QPointF(4, -5) << QPointF(0, 5);
        painter->drawPolygon(shirt);
    }

    // --- Text Details ---
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

void WorkerItem::setAnimationSpeed(float multiplier)
{
    if (multiplier <= 0) multiplier = 1.0f;
    animation_->setDuration(static_cast<int>(600 / multiplier));
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
