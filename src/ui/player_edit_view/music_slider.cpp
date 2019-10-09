#include "music_slider.h"

Music_slider::Music_slider(int h)
    : m_height(h)
    , m_time(0)
{
}


void Music_slider::set_height(int height)
{
    m_height = height;
}


QRectF Music_slider::boundingRect() const
{
    return QRectF(-5, 0, 10, m_height);
}


void Music_slider::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option);
    Q_UNUSED(widget);

    painter->setBrush(QBrush(Qt::green, Qt::SolidPattern));
    painter->setPen(QPen(Qt::green, 1));
    QPolygonF CursorHead;
    CursorHead.append(QPointF(-5.0, 22.0));
    CursorHead.append(QPointF(5.0, 22.0));
    CursorHead.append(QPointF(5.0, 25.0));
    CursorHead.append(QPointF(0.0, 35.0));
    CursorHead.append(QPointF(-5.0, 25.0));
    CursorHead.append(QPointF(-5.0, 22.0));
    painter->drawPolygon(CursorHead);
    painter->setPen(Qt::NoPen);
    painter->drawRect(0, 35, 1, m_height - 35);
}

void Music_slider::set_time(quint32 t)
{
    m_time = t;
}

