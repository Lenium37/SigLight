#ifndef MUSIC_SLIDER_H
#define MUSIC_SLIDER_H

#include <QObject>
#include <QGraphicsItem>
#include <QPainter>

class Music_slider : public QGraphicsItem
{
    Q_INTERFACES(QGraphicsItem)

public:
    Music_slider(int h);

    QRectF boundingRect() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

    void set_height(int height);

    void position_changed(qint64 position);

    void set_time(quint32 t);

private:
    int m_height;
    quint32 m_time;


};

#endif // MUSIC_SLIDER_H
