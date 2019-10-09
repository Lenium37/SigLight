#ifndef SONG_LABEL_H
#define SONG_LABEL_H

#include <QGraphicsItem>
#include <QPainter>
#include <QWidget>


class SongLabel : public QGraphicsItem
{
    Q_INTERFACES(QGraphicsItem)

public:
    SongLabel(int _width, int _height, QString _song_name);

    QRectF boundingRect() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

private:
    int width;
    int height;
    QString song_name;

};

#endif // SONG_LABEL_H
