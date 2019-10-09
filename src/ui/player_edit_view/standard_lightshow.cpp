#include "standard_lightshow.h"

Standard_lightshow::Standard_lightshow(int object_width, int object_height)
    : width (object_width)
    , height (object_height)
{
}

QRectF Standard_lightshow::boundingRect() const
{
    return QRectF(0, 0, width, height);
}

void Standard_lightshow::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setBrush(QBrush(QColor(132, 132, 132, 250)));
    painter->setPen(QColor(132, 132, 132, 250));
    painter->drawRect(0, 0, width, height);
}

/* Editing thew lightshow has been shut down
void Standard_lightshow::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *)
{
    emit item_double_clicked(this);
}
*/
