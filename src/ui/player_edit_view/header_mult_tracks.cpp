#include "header_mult_tracks.h"

Header_mult_tracks::Header_mult_tracks(qint64 _song_duration)
    : song_duration(_song_duration)
    , time_step(HALF_SECOND_WIDTH)
{
    total_width = ((song_duration * 2) * HALF_SECOND_WIDTH) + 75;
}

QRectF Header_mult_tracks::boundingRect() const
{
    return QRectF(0, 0, total_width, HEADER_HEIGHT);
}

void Header_mult_tracks::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    painter->setPen(QPen(QColor(150, 150, 150, 255), 1));
    painter->setBrush(QBrush(QColor(150, 150, 150, 255)));
    painter->drawRect(0, 0, total_width, HEADER_HEIGHT);

    for (int i = 0; i < total_width / HALF_SECOND_WIDTH; i++){
        float xpos = (i * HALF_SECOND_WIDTH);
        int seconds = (int) i / 2;
        int minutes = seconds / 60;
        seconds -= minutes * 60;

        QTime t(0, minutes, seconds);
        if (i % 2 == 0){
            painter->setPen(QPen(QColor(5, 255, 5, 255), 1));
            painter->drawLine(xpos, 20, xpos, 34);
            if (i % 2 == 0 && i != 0){
                painter->drawText(xpos - 15, 17, t.toString("mm:ss"));
            }
        } else {
            painter->setPen(QPen( QColor(250, 250, 250, 255), 1));
            painter->drawLine(xpos, 25, xpos, 34);
        }
    }

}

/* Changing Position in the song has been shut down, because of time reasons
void Header_mult_tracks::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    emit header_double_clicked(event->pos().x());
}
*/
