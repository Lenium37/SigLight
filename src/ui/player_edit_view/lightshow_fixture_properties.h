#ifndef LIGHTSHOW_FIXTURE_PROPERTIES_H
#define LIGHTSHOW_FIXTURE_PROPERTIES_H

#include <QGraphicsItem>
#include <QObject>
#include <QPainter>
#include <QLabel>

class Lightshow_fixture_properties : public QGraphicsItem
{
    Q_INTERFACES(QGraphicsItem)

public:
    Lightshow_fixture_properties (QString fixture_name, int properties_width, int properties_height);

private:
    const QString name;
    int fixture_index;
    bool is_active_changing;
    int width;
    int height;

    QRectF boundingRect() const;

    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget);

};

#endif // LIGHTSHOW_FIXTURE_PROPERTIES_H
