#ifndef HEADER_MULT_TRACKS_H
#define HEADER_MULT_TRACKS_H

#include <QGraphicsItem>
#include <QPainter>
#include <QTime>
#include <QGraphicsSceneMouseEvent>

#define HEADER_HEIGHT       35
#define HALF_SECOND_WIDTH   25



class Header_mult_tracks : public QGraphicsItem
{
    Q_INTERFACES(QGraphicsItem)

public:
    Header_mult_tracks(qint64 _song_duration);

    QRectF boundingRect() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

protected slots:
    // Changing Position in the song has been shut down, because of time reasons
    // void mouseDoubleClickEvent (QGraphicsSceneMouseEvent *event);


signals:
     void header_double_clicked(double x_pos_of_click);

private:
    int total_width;
    qint64 song_duration;
    float time_step;
};

#endif // HEADER_MULT_TRACKS_H
