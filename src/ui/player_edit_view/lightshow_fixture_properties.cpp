#include "lightshow_fixture_properties.h"

Lightshow_fixture_properties::Lightshow_fixture_properties(QString fixture_name, int properties_width, int properties_height)
    : name(fixture_name)
    , width(properties_width)
    , height(properties_height)
{
}


QRectF Lightshow_fixture_properties::boundingRect() const
{
    return QRectF(0, 0, width, height);
}

void Lightshow_fixture_properties::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setBrush(QBrush(QColor(112, 127, 98, 255)));
    painter->setPen(QPen(QColor(112, 127, 98, 255)));
    painter->drawRect(0, 0, width, height);

    painter->setPen(QPen(QColor(250, 250, 250, 255)));
    painter->setFont(QFont("times", 22));
    painter->drawText(5, 65, this->name);

    /* if we need channels
    for (int i = 1; i < channel_count; i++){
        painter->setPen(QPen(QColor(255, 69, 0, 255), 1));
        painter->drawLine(200, i * 21, 250, i * 21);
    }

    for (int i = 1; i <= channel_count; i++){
        painter->setPen(QPen(QColor(240, 250, 250, 255), 1));
        QString c = "Channel " + QString::number(i);
        QRectF textRect = QRect(200, (i - 1) * 21, 50, 21);
        painter->drawText(textRect, Qt::AlignBottom, c);
    }
    */
}




