#include "song_label.h"

SongLabel::SongLabel(int _width, int _height, QString _song_name)
    : width(_width)
    , height(_height)
    , song_name(_song_name)
{    
}

QRectF SongLabel::boundingRect() const
{
    return QRectF(0, 0, width, height);
}

void SongLabel::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setBrush(QBrush(QColor(150, 150, 150, 255)));
    painter->setPen(QPen(QColor(150, 150, 150, 255)));
    painter->drawRect(0, 0, width, height);

    painter->setPen(QPen(QColor(250, 250, 250, 255)));
    painter->setFont(QFont("times", 10));
    painter->drawText(10, 25, this->song_name);
}

